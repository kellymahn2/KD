#include "EditorLayer.h"


#include "Kaidel/Math/Math.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/RenderPass.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Kaidel/Scene/SceneSerializer.h"

#include "Kaidel/Utils/PlatformUtils.h"

#include "Kaidel/Scripting/ScriptEngine.h"


#include "imguizmo/ImGuizmo.h"


namespace Kaidel {
	glm::vec4 _GetUVs();
	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		KD_PROFILE_FUNCTION();
		m_Icons.IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
		KD_INFO("Loaded Play Button");
		m_Icons.IconPause = Texture2D::Create("Resources/Icons/PauseButton.png");
		KD_INFO("Loaded Pause Button");
		m_Icons.IconSimulateStart = Texture2D::Create("Resources/Icons/SimulateButtonStart.png");
		KD_INFO("Loaded Simulation Play Button");
		m_Icons.IconSimulateStop = Texture2D::Create("Resources/Icons/SimulateButtonStop.png");
		KD_INFO("Loaded Simulation Stop Button");
		m_Icons.IconStop = Texture2D::Create("Resources/Icons/StopButton.png");
		KD_INFO("Loaded Stop Button");

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8,{FramebufferTextureFormat::RED_INTEGER,true}, FramebufferTextureFormat::DEPTH32 };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		m_Framebuffer = Framebuffer::Create(fbSpec);
		m_ActiveScene = CreateRef<Scene>();
		m_EditorScene = m_ActiveScene;
		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		auto& commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1) {
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
			//OpenScene(sceneFilePath);
		}
		else {
			//TODO: Actually create a new project
			NewProject();
		}

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_SceneHierarchyPanel.RegisterFieldRenderers();
		m_ConsolePanel.SetContext(::Log::GetClientLogger());


		{
			Entity e = m_ActiveScene->CreateEntity("Light");
			auto& plc  = e.AddComponent<SpotLightComponent>();
			
		}
		{
			Entity e = m_ActiveScene->CreateEntity("Cube");
			auto& crc = e.AddComponent<CubeRendererComponent>();
			crc.Material = CreateRef<Material>();
			crc.Material->SetDiffuse(2);
			crc.Material->SetSpecular(3);
			//crc.Color = glm::vec4(1.0f);
		}


		/*cs = ComputeShader::Create("assets/shaders/TestCompute2.glsl");

		tbi = TypedBufferInput::Create(TypedBufferInputDataType::RGBA8, 799, 449);
		ub = UniformBuffer::Create(4, 0);*/
		/*float x = 1.0f;
		ui = UAVInput::Create(sizeof(x), &x);*/
		
		/*{
			auto e = m_ActiveScene->CreateEntity();
			e.AddComponent<CubeRendererComponent>();
		}
		{
			auto e = m_ActiveScene->CreateEntity();
			e.AddComponent<SpriteRendererComponent>();
			e.GetComponent<TransformComponent>().Translation = glm::vec3{ 1.0 };
		}*/
		/*{
			auto e = m_ActiveScene->CreateEntity();
			auto& crc = e.AddComponent<CubeRendererComponent>();
			crc.Color = { .8,.6,.3,1.0 };
		}*/
		/*Entity e = m_ActiveScene->CreateEntity();*/

		/*a = CreateRef<TransformAnimationFrame>(glm::vec3{ 1,1,0 }, 2.0f);
		ac = CreateRef<AnimationController>();*/
		/*a = CreateRef<Animation>();
		ap = CreateRef<AnimationPlayer>();*/

		/*auto property = a->AddProperty(AnimatedPropertyType::Translate);
		KeyFrame<AnimatedPropertyType::Translate> kf1{};
		kf1.StartTime = 0.0f;
		kf1.EndTime = 5.0f;
		kf1.TargetTranslation = { 1.0f,1.0f,.0 };
		KeyFrame<AnimatedPropertyType::Translate> kf2{};
		kf2.StartTime = 5.0f;
		kf2.EndTime = 8.0f;
		kf2.TargetTranslation = { 2,1,0 };
		property->AddKeyFrame<AnimatedPropertyType::Translate>(kf1);
		property->AddKeyFrame<AnimatedPropertyType::Translate>(kf2);
		auto& animationComponent = e.AddComponent<AnimationComponent>(a,ap);*/
	}

	void EditorLayer::OnDetach()
	{
		KD_PROFILE_FUNCTION();
	}

	static int GetCurrentPixelData(glm::vec2 viewportBounds[2],Ref<Framebuffer> fb) {
		auto [mx, my] = ImGui::GetMousePos();
		mx -= viewportBounds[0].x;
		my -= viewportBounds[0].y;
		glm::vec2 viewportSize = viewportBounds[1] - viewportBounds[0];
		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
			my = viewportSize.y - my; break;
		default:
			break;
		}
		int mouseX = (int)mx;
		int mouseY = (int)my;
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			return fb->ReadPixel(1, mouseX, mouseY);
		}
		return -1;
	}
	static float t = 0.0f;
	void EditorLayer::OnUpdate(Timestep ts)
	{
		KD_PROFILE_FUNCTION();

		
		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}
		// Update
		if(m_SceneState==SceneState::Edit)
			m_EditorCamera.OnUpdate(ts);
		

		/*{
			ub->SetData(&totalTime, 4, 0);
			ub->Bind();
			cs->SetTypedBufferInput(tbi, TypedBufferAccessMode::ReadWrite, 0);
			cs->SetUAVInput(ui,0);
			cs->Bind();
			cs->Execute(ceil(m_ViewportSize.x/8),ceil(m_ViewportSize.y/8), 1);
			cs->Wait();
			totalTime += ts;
		}*/


		// Render
		Renderer2D::ResetStats();
	

		/*{
			auto view = m_ActiveScene->m_Registry.view<TransformComponent, LightComponent>();
			Renderer2D::BeginScene(m_EditorCamera);
			for (auto e : view) {
				Renderer2D::DrawQuad(glm::translate(glm::mat4(1.0f),view.get<LightComponent>(e).Light->GetPosition()), 
					glm::vec4(1.0f), 0, -1);
			}
			Renderer2D::EndScene();
		}*/
		/*Renderer2D::BeginScene(m_EditorCamera);
		Renderer2D::DrawQuad(glm::translate(glm::mat4(1.0f), glm::vec3{ 0,0,0 }), {1,0,0,1}, 4, -1);
		Renderer2D::DrawQuad(glm::translate(glm::mat4(1.0f), glm::vec3{ .5f,.5f,-.5f }), { 0,1,0,1 }, 0, -1);

		Renderer2D::EndScene();*/


		//Renderer2D::BeginScene(m_EditorCamera);
		//Renderer2D::DrawQuad(glm::mat4(1.0f), { 0.6f,.8f,.5f,1.0f }, 0);
		//Renderer2D::EndScene();
		//glm::mat4 r = m_EditorCamera.GetProjection()* m_EditorCamera.GetViewMatrix();
		//ub->SetData(&r[0][0], sizeof(glm::mat4));
		//float p[] = {
		//	0.f, 0.5f, 0.0f,	.0f,.0f,1.0f,1.0f,
		//	0.5f, -0.5f, 0.0f,	1.0f,.0f,.0f,1.0f,
		//	-0.5f, -0.5f, 0.0f, .0f,1.0f,.0f,1.0f,
		//};
		//vb->SetData(p, sizeof(p));
		//vb->Bind();
		////s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
		//ib->Bind();
		//ub->Bind();
		//va->Bind();
		//s->Bind();
		//RenderCommand::DrawIndexed(va, 3);
		// Update scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{

				{
					m_Framebuffer->Bind();
					float c[4] = { .1f, .1f,.1f, 1 };
					m_Framebuffer->ClearAttachment(0, c);
					float d[4] = { -1,-1,-1,-1 };
					m_Framebuffer->ClearAttachment(1, d);
					GeometryPass goemetryPass = { GeometryPass::GeometryType_3D | GeometryPass::GeometryType_2D , {m_ActiveScene,m_EditorCamera.GetViewProjection(),m_EditorCamera.GetPosition()} };
					goemetryPass.Render();
					m_Framebuffer->Unbind();

					ShadowPass shadowPass{ {m_ActiveScene} };
					shadowPass.Render();

				}

				{
						
				}

				{


				}

				{



				}

				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRuntime(ts);
				break;
			}
			case SceneState::Simulate:
			{
				m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
				break;
			}
		}
		
		if (m_Debug) {
			OnOverlayRender();
		}
		
		if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity(); selectedEntity && m_SceneState == SceneState::Edit)
		{
			DrawSelectedEntityOutline(selectedEntity);
		}
		//int pixelData = GetCurrentPixelData(m_ViewportBounds,m_Framebuffer);
		//m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
	}

	void EditorLayer::OnOverlayRender()
	{
		if(m_SceneState == SceneState::Edit)
			Renderer2D::BeginScene(m_EditorCamera);
		else if (m_SceneState == SceneState::Play)
		{
			Entity camera = m_ActiveScene->GetPrimaryCameraEntity();
			Renderer2D::BeginScene(camera.GetComponent<CameraComponent>().Camera, camera.GetComponent<TransformComponent>().GetTransform());
		}
		{
			auto view = m_ActiveScene->GetAllComponentsWith<TransformComponent, CircleCollider2DComponent>();
			for (auto e : view) {
				
				auto [tc, bc2d] = view.get<TransformComponent, CircleCollider2DComponent>(e);
				auto scale = tc.Scale * glm::vec3(bc2d.Radius * 2.0f, bc2d.Radius * 2.0f, 1.0f);
				auto transform = glm::translate(glm::mat4(1.0f), tc.Translation)
					* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.0f))
					* glm::scale(glm::mat4(1.0f), scale+0.01f);
				Renderer2D::DrawCircle(transform, { .2f,.9f,.3f,1.0f }, .02f);
			}
		}
		{
			auto view = m_ActiveScene->GetAllComponentsWith<TransformComponent, BoxCollider2DComponent>();
			for (auto e : view) {

				auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(e);
				auto scale = tc.Scale * glm::vec3(bc2d.Size*2.0f, 1.0f);
				auto transform = glm::translate(glm::mat4(1.0f), tc.Translation)
					* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
					*glm::translate(glm::mat4(1.0f),glm::vec3(bc2d.Offset,0.0f))
					* glm::scale(glm::mat4(1.0f), scale+.01f);
				Renderer2D::DrawRect(transform, {.2f,.9f,.3f,1.0f});
			}
		}
		Renderer2D::EndScene();
	}
	

	
	void EditorLayer::OnImGuiRender()
	{
		KD_PROFILE_FUNCTION();
		
		// Note: Switch this to true to enable dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_PassthruCentralNode;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 200.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
				if (ImGui::MenuItem("Duplicate","Ctrl+D"))
					m_ActiveScene->DuplicateEntity(m_SceneHierarchyPanel.GetSelectedEntity());
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				if (ImGui::MenuItem("Save", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				if (ImGui::MenuItem("Exit")) 
					Application::Get().Close();

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Script")) {
				if (ImGui::MenuItem("Reload Assembly"))
					ScriptEngine::ReloadAssembly();

				ImGui::EndMenu();
			}


			ImGui::EndMenuBar();
		}
		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_ConsolePanel.OnImGuiRender();
		ShowDebugWindow();
		ShowViewport();
		UI_Toolbar();
		ImGui::End();

		if (m_ConsoleOpen) {
			ImGui::Begin("Debug Console", &m_ConsoleOpen,ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNavFocus);
			static bool core = true;
			static bool client = true;
			ImGui::Checkbox("Core", &core);
			ImGui::Checkbox("Client", &client);
			if(core){
				for (auto& message : ::Log::GetCoreLogger()->GetMessages()) {

					ImVec4 messageColor{ 1,1,1,1 };
					switch (message.Level)

					{

					case MessageLevel::Info:
					{
						messageColor = { .24f,.71f,.78f,1.0f };
					}
					break;
					case MessageLevel::Warn:
					{
						messageColor = { .79f,.78f,.32f,1.0f };
					}
					break;
					case MessageLevel::Error:
					{
						messageColor = { .65f,.31f,.29f,1.0f };
					}
					break;
					default:
						break;
					}
					std::time_t time = std::chrono::system_clock::to_time_t(message.Time);
					std::tm tm = *std::localtime(&time);
					char buf[80] = { 0 };
					std::strftime(buf, sizeof(buf), "%d/%m/%Y-%H:%M:%S", &tm);
					ImGui::TextColored(messageColor, "Engine [%s] : %s",buf, message.Text.c_str());
				}
			}
			if (client) {
				for (auto& message : ::Log::GetClientLogger()->GetMessages()) {

					ImVec4 messageColor{ 1,1,1,1 };
					switch (message.Level)

					{

					case MessageLevel::Info:
					{
						messageColor = { .24f,.71f,.78f,1.0f };
					}
					break;
					case MessageLevel::Warn:
					{
						messageColor = { .79f,.78f,.32f,1.0f };
					}
					break;
					case MessageLevel::Error:
					{
						messageColor = { .65f,.31f,.29f,1.0f };
					}
					break;
					default:
						break;

					}
					std::time_t time = std::chrono::system_clock::to_time_t(message.Time);
					std::tm tm = *std::localtime(&time);
					char buf[80] = { 0 };
					std::strftime(buf, sizeof(buf), "%d/%m/%Y-%H:%M:%S", &tm);
					ImGui::TextColored(messageColor, "Editor [%s] : %s",buf ,message.Text.c_str());
				}
			}
			ImGui::End();
		}
	}

	void EditorLayer::OnScenePlay(){
		if (!m_EditorScene||!m_ActiveScene->GetPrimaryCameraEntity())
			return;
		m_SceneState = SceneState::Play;
		m_RuntimeScene =Scene::Copy(m_EditorScene);
		m_RuntimeScene->OnRuntimeStart();
		m_ActiveScene = m_RuntimeScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OnSceneStop(){
		m_SceneState = SceneState::Edit;
		m_ActiveScene->OnRuntimeStop();
		m_RuntimeScene = nullptr;
		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	

	void EditorLayer::OnSceneSimulateStart() {
		if (!m_EditorScene)
			return;
		if (m_SceneState == SceneState::Play)
			OnSceneStop();
		m_SceneState = SceneState::Simulate;
		m_SimulationScene = Scene::Copy(m_EditorScene);
		m_SimulationScene->OnSimulationStart();
		m_ActiveScene = m_SimulationScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}
	void EditorLayer::OnSceneSimulateStop() {
		m_SceneState = SceneState::Edit;
		m_SimulationScene->OnSimulationStop();
		m_SimulationScene = nullptr;
		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	
	EditorLayer::GizmoCamera EditorLayer::GetCurrentCameraViewProjection()
	{
		if (m_SceneState == SceneState::Edit)
			return { m_EditorCamera.GetViewMatrix(),m_EditorCamera.GetProjection()};
		else if (m_SceneState == SceneState::Play)
		{
			auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			return { glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform()),camera.GetProjection()};
		}
	}

	void EditorLayer::MoveChildren(Entity curr, const glm::vec3& deltaTranslation, const glm::vec3& deltaRotation, Entity parent ) {
		auto& tc = curr.GetComponent<TransformComponent>();
		if (curr.HasComponent<ParentComponent>()) {
			for (auto& child : curr.GetComponent<ParentComponent>().Children) {
				auto entity = m_ActiveScene->GetEntity(child);
				MoveChildren(entity, deltaTranslation, deltaRotation, parent ? parent : curr);
			}
		}
		if (parent && (deltaRotation.x || deltaRotation.y || deltaRotation.z)) {
			Math::Rotate(curr, parent, deltaRotation);
		}
		else {
			if(parent)
			tc.Rotation += deltaRotation;
		}
		tc.Translation += deltaTranslation;
	}
	
	void EditorLayer::DrawGizmos()
	{

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			// Camera

			// Runtime camera from entity
			auto gizmoCamera = GetCurrentCameraViewProjection();
			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyDown(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(gizmoCamera.View), glm::value_ptr(gizmoCamera.Projection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);
				glm::vec3 deltaTranslation = translation - tc.Translation;
				glm::vec3 deltaRotation = rotation - tc.Rotation;
				for (int i = 0; i < 3; ++i) {
					if (glm::epsilonEqual(deltaRotation[i], 0.0f, glm::epsilon<float>())) {
						deltaRotation[i] = 0.0f;
					}
				}
				for (int i = 0; i < 3; ++i) {
					if (glm::epsilonEqual(deltaTranslation[i], 0.0f, glm::epsilon<float>())) {
						deltaTranslation[i] = 0.0f;
					}
				}
				MoveEntity(selectedEntity, m_ActiveScene.get(), deltaTranslation, deltaRotation);
				tc.Scale = scale;
			}
		}
	}

	void EditorLayer::DrawSelectedEntityOutline(Entity selectedEntity) {
		Renderer2D::BeginScene(m_EditorCamera);
		//Renderer2D::SetLineWidth(4.0f);
		auto& tc = selectedEntity.GetComponent<TransformComponent>();
		if (selectedEntity.HasComponent<SpriteRendererComponent>()) {

			auto& pos = tc.Translation;
			auto& rot = glm::toMat4(glm::quat(tc.Rotation));
			auto& scale = tc.Scale + .02f;
			auto& col = selectedEntity.GetComponent<SpriteRendererComponent>().Color;
			auto transform = glm::translate(glm::mat4(1.0f), pos) * rot * glm::scale(glm::mat4(1.0f), scale);
			if (col == glm::vec4{ 1,0,0,1 })
				Renderer2D::DrawRect(transform, glm::vec4{ 1 });
			else
				Renderer2D::DrawRect(transform, glm::vec4{ 1,0,0,1 });
		}
		else if (selectedEntity.HasComponent<CircleRendererComponent>()) {
			auto& crc = selectedEntity.GetComponent<CircleRendererComponent>();

			auto scale = tc.Scale;
			auto transform = glm::translate(glm::mat4(1.0f), tc.Translation)
				* glm::scale(glm::mat4(1.0f), scale + 0.01f);
			auto& col = selectedEntity.GetComponent<CircleRendererComponent>().Color;
			if (col == glm::vec4(1, 0, 0, 1))
				Renderer2D::DrawCircle(transform, glm::vec4{ 1 }, .02f);
			else
				Renderer2D::DrawCircle(transform, glm::vec4{ 1,0,0,1 }, .02f);
		}
		Renderer2D::EndScene();
	}

	void EditorLayer::ShowDebugWindow()
	{
		ImGui::Begin("Styler");
		ImGui::ShowStyleEditor();
		ImGui::End();
		ImGui::Begin("Stats");

		std::string name = "None";
		//if (m_HoveredEntity)
			//name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		ImGui::Text("Hovered Entity: %s, %d", name.c_str(),m_HoveredEntity.operator entt::id_type());
		ImGui::Text("Gizmo Mode : %d", m_GizmoType);
		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Frame Rate: %.3f", ImGui::GetIO().Framerate);
		ImGui::Text("Selected Entity: %d", m_SceneHierarchyPanel.GetSelectedEntity().operator entt::entity());
		ImGui::Text("%f,%f", m_ViewportSize.x, m_ViewportSize.y);
		//ImGui::Text("%f", a->GetTime());
		auto t = m_Icons.IconStop;
		ImGui::Image((void*)t->GetRendererID(), ImVec2{ (float)t->GetWidth(),(float)t->GetHeight() });
		ImGui::End();

	}
	static void UpdateBounds(glm::vec2 bounds[2]) {
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		bounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		bounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
	}
	void EditorLayer::ShowViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", nullptr, 0 | ImGuiWindowFlags_NoTitleBar);

		UpdateBounds(m_ViewportBounds);
		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		auto textureID = m_Framebuffer->GetColorAttachmentRendererID();
		glm::vec4 uvs = _GetUVs();
		ImGui::Image(reinterpret_cast<void*>(m_Framebuffer->GetColorAttachmentRendererID()), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, { uvs.x,uvs.y }, { uvs.z,uvs.w });
		if (ImGui::BeginDragDropTarget()) {
			if (auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(path);
			}
			ImGui::EndDragDropTarget();
		}
		// Gizmos
		DrawGizmos();


		ImGui::End();
		ImGui::PopStyleVar();
	}



	void EditorLayer::UI_Toolbar() {
		constexpr auto windowFlags =
			ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoTitleBar;
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,2 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0,2 });
		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { buttonHovered.x,buttonHovered.y,buttonHovered.z,.5f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { buttonActive.x,buttonActive.y,buttonActive.z,.5f });

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::Begin("##toolbar", nullptr, windowFlags);

		float size = ImGui::GetWindowHeight() - 4.0f;

		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * .5f) - (size * .5f));
		ImGui::SetCursorPosY((ImGui::GetWindowContentRegionMax().y * .5f) - (size * .5f));

		{
			Ref<Texture2D> icon = m_Icons.IconPlay;
			if (m_SceneState == SceneState::Play)
				icon = m_Icons.IconStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { size,size }, { 0,0 }, { 1,1 }, 0)) {
				if (m_SceneState == SceneState::Edit) {
					OnScenePlay();
				}
				else if (m_SceneState == SceneState::Play) {
					OnSceneStop();
				}
				else if (m_SceneState == SceneState::Simulate) {
					OnSceneSimulateStop();
					OnScenePlay();
				}
			}
		}
		{
			if (m_SceneState == SceneState::Play) {
				ImGui::SameLine();
				Ref<Texture2D> icon = m_Icons.IconPause;
				if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { size,size }, { 0,0 }, { 1,1 }, 0)){
					m_ActiveScene->ChangePauseState();

				}
			}
		}
		ImGui::SameLine();
		{
			Ref<Texture2D> icon = m_Icons.IconSimulateStart;
			if (m_SceneState == SceneState::Simulate)
				icon = m_Icons.IconSimulateStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { size,size }, { 0,0 }, { 1,1 }, 0)) {
				if (m_SceneState == SceneState::Simulate)
					OnSceneSimulateStop();
				else
					OnSceneSimulateStart();
			}
		}

		ImGui::End();
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(3);
	}







	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(KD_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(KD_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// Shortcuts
		if (e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyDown(Key::LeftControl) || Input::IsKeyDown(Key::RightControl);
		bool shift = Input::IsKeyDown(Key::LeftShift) || Input::IsKeyDown(Key::RightShift);

		switch (e.GetKeyCode())
		{
			case Key::N:
			{
				if (control)
					NewScene();

				break;
			}
			case Key::O:
			{
				if (control)
					OpenScene();

				break;
			}
			case Key::S:
			{

				if (control && shift)
					SaveSceneAs();
				else if (control)
					SaveScene();
				break;
			}

			case Key::D:
			{
				if (control&&!shift)
					m_ActiveScene->DuplicateEntity(m_SceneHierarchyPanel.GetSelectedEntity());
				else if (control&&shift)
					m_Debug = !m_Debug;
				break;
			}
			// Gizmos
			case Key::Q:
			{

				if (!ImGuizmo::IsUsing() && m_SceneState != SceneState::Play)
					m_GizmoType = -1;
				break;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing() && m_SceneState != SceneState::Play)
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing() && m_SceneState != SceneState::Play)
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			}
			case Key::R:
			{
				if (!ImGuizmo::IsUsing() && m_SceneState != SceneState::Play)
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			}
			case Key::GraveAccent:
			{
				if (control) {
					m_ConsoleOpen = !m_ConsoleOpen;
				}
			}
		}
		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::ButtonLeft)
		{
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyDown(Key::LeftAlt)) {
				m_Framebuffer->Bind();
				int pixel = GetCurrentPixelData(m_ViewportBounds, m_Framebuffer);
				m_SceneHierarchyPanel.SetSelectedEntity(pixel == -1 ? Entity{} : Entity{(entt::entity) pixel,m_ActiveScene.get() });
				m_Framebuffer->Unbind();
			}
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();
		std::optional<std::string> filepath = FileDialogs::OpenFile("Kaidel Scene (*.Kaidel)\0*.Kaidel\0");
		if (filepath)
		{
			OpenScene(*filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();
		if (path.extension().string() != ".Kaidel" && path.extension().string() != ".kaidel")
		{
			KD_WARN("Could not load {0} - not a scene file", path.filename().string());
		}
		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		newScene->SetPath(path.string());
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(path.string())) {
			m_EditorScene = newScene;

			m_SceneHierarchyPanel.SetContext(m_EditorScene);
			m_ActiveScene = m_EditorScene;
		}
	}

	void EditorLayer::SaveScene()
	{
		SceneSerializer serializer(m_EditorScene);
		serializer.Serialize(m_EditorScene->GetPath());
	}

	void EditorLayer::SaveSceneAs()
	{
		std::optional<std::string> filepath = FileDialogs::SaveFile("Kaidel Scene (*.Kaidel)\0*.Kaidel\0");
		if (filepath)
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(*filepath);
		}
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path)) {
			auto startScenePath = Project::GetAssetPath(Project::GetActive()->GetConfig().StartScene);
			OpenScene(startScenePath);
		}
	}

	void EditorLayer::SaveProject()
	{
	}

}
