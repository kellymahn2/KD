#include "EditorLayer.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Kaidel/Scene/SceneSerializer.h"

#include "Kaidel/Utils/PlatformUtils.h"

#include "imguizmo/ImGuizmo.h"

#include "Kaidel/Math/Math.h"
#include "Kaidel/Core/Timer.h"
namespace Kaidel {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f), m_SquareColor({ 0.2f, 0.3f, 0.8f, 1.0f })
	{
	}

	void EditorLayer::OnAttach()
	{
		KD_PROFILE_FUNCTION();
		m_CheckerboardTexture = Texture2D::Create("assets/textures/Checkerboard.png");
		m_Icons.IconPlay = Texture2D::Create("Resources/Icons/PlayButton.png");
		m_Icons.IconSimulateStart = Texture2D::Create("Resources/Icons/SimulateButtonStart.png");
		m_Icons.IconSimulateStop = Texture2D::Create("Resources/Icons/SimulateButtonStop.png");
		m_Icons.IconStop = Texture2D::Create("Resources/Icons/StopButton.png");

		FramebufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		m_Framebuffer = Framebuffer::Create(fbSpec);
		m_ActiveScene = CreateRef<Scene>();

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

#if 0
		// Entity
		auto square = m_ActiveScene->CreateEntity("Green Square");
		square.AddComponent<SpriteRendererComponent>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});

		auto redSquare = m_ActiveScene->CreateEntity("Red Square");
		redSquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

		m_SquareEntity = square;

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera A");
		m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCamera = m_ActiveScene->CreateEntity("Camera B");
		auto& cc = m_SecondCamera.AddComponent<CameraComponent>();
		cc.Primary = false;

		class CameraController : public ScriptableEntity
		{
		public:
			virtual void OnCreate() override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;
				translation.x = rand() % 10 - 5.0f;
			}

			virtual void OnDestroy() override
			{
			}

			virtual void OnUpdate(Timestep ts) override
			{
				auto& translation = GetComponent<TransformComponent>().Translation;

				float speed = 5.0f;

				if (Input::IsKeyPressed(Key::A))
					translation.x -= speed * ts;
				if (Input::IsKeyPressed(Key::D))
					translation.x += speed * ts;
				if (Input::IsKeyPressed(Key::W))
					translation.y += speed * ts;
				if (Input::IsKeyPressed(Key::S))
					translation.y -= speed * ts;
			}
		};

		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
		m_SecondCamera.AddComponent<NativeScriptComponent>().Bind<CameraController>();
#endif

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			return fb->ReadPixel(1, mouseX, mouseY);
		}
		return -1;
	}
	void EditorLayer::OnUpdate(Timestep ts)
	{
		KD_PROFILE_FUNCTION();

		// Resize
		if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Update
		m_EditorCamera.OnUpdate(ts);

		// Render
		Renderer2D::ResetStats();
		m_Framebuffer->Bind();

		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		// Clear our entity ID attachment to -1
		m_Framebuffer->ClearAttachment(1, -1);


		// Update scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
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
		
		if (Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();selectedEntity&&m_SceneState==SceneState::Edit)
		{

			Renderer2D::BeginScene(m_EditorCamera);
			Renderer2D::SetLineWidth(4.0f);
			auto& tc= selectedEntity.GetComponent<TransformComponent>();
			if (selectedEntity.HasComponent<SpriteRendererComponent>()) {
				auto pos = tc.GetTransform();
				auto& col=selectedEntity.GetComponent<SpriteRendererComponent>().Color;

				if(col==glm::vec4{ 1,0,0,1 })
					Renderer2D::DrawRect(pos,glm::vec4{1});
				else 
					Renderer2D::DrawRect(pos, glm::vec4{ 1,0,0,1 });
			}
			else if (selectedEntity.HasComponent<CircleRendererComponent>()) {
				auto pos = glm::scale(tc.GetTransform(), glm::vec3(1.02f, 1.02f, 1.0f));
				auto& col = selectedEntity.GetComponent<CircleRendererComponent>().Color;
				if(col==glm::vec4(1, 0, 0, 1))
					Renderer2D::DrawCircle(pos, glm::vec4{1}, .02f);
				else 
					Renderer2D::DrawCircle(pos, glm::vec4{ 1,0,0,1 }, .02f);
			}
			Renderer2D::EndScene();
		}
		int pixelData = GetCurrentPixelData(m_ViewportBounds,m_Framebuffer);
		m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());

		m_Framebuffer->Unbind();
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
				auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(e);
				auto pos = glm::scale(tc.GetTransform(), glm::vec3(1.02f, 1.02f, 1.0f));
				Renderer2D::DrawCircle(pos, { .2f,.9f,.3f,1.0f }, .02f);
			}
		}
		{
			auto view = m_ActiveScene->GetAllComponentsWith<TransformComponent, BoxCollider2DComponent>();
			for (auto e : view) {
				auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(e);
				auto scale = tc.Scale * glm::vec3(bc2d.Size*2.0f, 1.0f);
				auto transform = glm::translate(glm::mat4(1.0f), tc.Translation)
					* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
					*glm::translate(glm::mat4(1.0f),glm::vec3(bc2d.Offset,0.001f))
					* glm::scale(glm::mat4(1.0f), scale);
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
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

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
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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
		style.WindowMinSize.x = 370.0f;
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

			ImGui::EndMenuBar();
		}

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();

		ShowDebugWindow();
		ShowViewport();
		UI_Toolbar();
		ImGui::End();
	}

	void EditorLayer::OnScenePlay(){
		if (!m_EditorScene)
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
			auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			//const glm::mat4& cameraProjection = camera.GetProjection();
			//glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera
			const glm::mat4& cameraProjection = m_SceneState == SceneState::Edit ? m_EditorCamera.GetProjection() : camera.GetProjection();

			glm::mat4 cameraView = m_SceneState == SceneState::Edit ? m_EditorCamera.GetViewMatrix() : glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;
			}
		}
	}


	void EditorLayer::ShowDebugWindow()
	{
		ImGui::Begin("Stats");

		std::string name = "None";
		if (m_HoveredEntity)
			name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
		ImGui::Text("Hovered Entity: %s, %d", name.c_str(),m_HoveredEntity.operator entt::id_type());
		ImGui::Text("Gizmo Mode : %d", m_GizmoType);
		auto stats = Renderer2D::GetStats();
		ImGui::Text("Renderer2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quads: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
		ImGui::Text("Frame Rate: %.3f", ImGui::GetIO().Framerate);
		ImGui::End();

	}

	void EditorLayer::ShowViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

		uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
		ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
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
			| ImGuiWindowFlags_NoCollapse;
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,2 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0,2 });
		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { buttonHovered.x,buttonHovered.y,buttonHovered.z,.5f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { buttonActive.x,buttonActive.y,buttonActive.z,.5f });

		ImGui::Begin("##toolbar", nullptr, windowFlags);


		float size = ImGui::GetWindowHeight() - 4.0f;

		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * .5f) - (size * .5f));
		ImGui::SetCursorPosY((ImGui::GetWindowContentRegionMax().y * .5f) - (size * .5f));

		{
			Ref<Texture2D> icon = m_Icons.IconPlay;
			if (m_SceneState == SceneState::Play)
				icon = m_Icons.IconStop;
			if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { size,size }, { 0,0 }, { 1,1 }, 0)) {
				if (m_SceneState == SceneState::Edit)
					OnScenePlay();
				else if (m_SceneState == SceneState::Play)
					OnSceneStop();
				else if (m_SceneState == SceneState::Simulate) {
					OnSceneSimulateStop();
					OnScenePlay();
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
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}







	void EditorLayer::OnEvent(Event& e)
	{
		m_CameraController.OnEvent(e);
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

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

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
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			}
			case Key::W:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}
			case Key::E:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			}
			case Key::R:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			}
		}
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == Mouse::ButtonLeft)
		{
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
				m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
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

}
