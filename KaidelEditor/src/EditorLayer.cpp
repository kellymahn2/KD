#include "EditorLayer.h"

#include "Kaidel/Math/Math.h"
#include "Kaidel/Core/Timer.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Kaidel/Scene/SceneSerializer.h"

#include "Kaidel/Utils/PlatformUtils.h"

#include "Kaidel/Scripting/ScriptEngine.h"

#include "yaml-cpp/yaml.h"

#include "imguizmo/ImGuizmo.h"
#include "Kaidel/Scene/SceneRenderer.h"


#include <forward_list>

#include <random>

namespace Kaidel {
	glm::vec4 _GetUVs();
	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}
	static std::vector<uint8_t> ReadFile(const FileSystem::path& filePath) {
		std::ifstream file(filePath, std::ios::binary | std::ios::in);
		std::vector<uint8_t> res;
		KD_CORE_ASSERT(file, "Could not read from file");
		file.seekg(0, std::ios::end);
		uint64_t size = file.tellg();
		KD_CORE_ASSERT(size != -1, "Could not read from file");

		res.resize(size);

		file.seekg(std::ios::beg);

		char* data = (char*)&res[0];
		file.read(data, size);
		return res;
	}
	
	void EditorLayer::OnAttach()
	{
		{
			SamplerState params{};
			params.MipFilter = SamplerMipMapMode::Linear;
			params.MinFilter = SamplerFilter::Linear;
			params.MagFilter = SamplerFilter::Linear;
			params.BorderColor = SamplerBorderColor::None;
			params.AddressModeU = SamplerAddressMode::ClampToEdge;
			params.AddressModeV = SamplerAddressMode::ClampToEdge;
			params.AddressModeW = SamplerAddressMode::ClampToEdge;
			m_OutputSampler = Sampler::Create(params);
		}
		
		m_Icons.IconPlay = EditorIcon("Resources/Icons/PlayButton.png",m_OutputSampler);
		KD_INFO("Loaded Play Button");
		m_Icons.IconPause = EditorIcon("Resources/Icons/PauseButton.png", m_OutputSampler);
		KD_INFO("Loaded Pause Button");
		m_Icons.IconSimulateStart = EditorIcon("Resources/Icons/SimulateButtonStart.png", m_OutputSampler);
		KD_INFO("Loaded Simulation Play Button");
		m_Icons.IconSimulateStop = EditorIcon("Resources/Icons/SimulateButtonStop.png", m_OutputSampler);
		KD_INFO("Loaded Simulation Stop Button");
		m_Icons.IconStop = EditorIcon("Resources/Icons/StopButton.png", m_OutputSampler);
		KD_INFO("Loaded Stop Button");

		{
			RenderPassSpecification specs{};
			RenderPassAttachment attach = RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::General, TextureSamples::x1);
			attach.LoadOp = AttachmentLoadOp::Clear;
			specs.Colors.push_back(attach);
			m_OutputRenderPass = RenderPass::Create(specs);
		}

		for(auto& texture : m_OutputTextures) {
			Texture2DSpecification specs{};
			specs.Width = 1280;
			specs.Height = 720;
			specs.Depth = 1;
			specs.Layers = 1;
			specs.Mips = 1;
			specs.Layout = ImageLayout::General;
			specs.Samples = TextureSamples::x1;
			specs.Format = Format::RGBA8UN;
			specs.Swizzles[0] = TextureSwizzle::Red;
			specs.Swizzles[1] = TextureSwizzle::Green;
			specs.Swizzles[2] = TextureSwizzle::Blue;
			specs.Swizzles[3] = TextureSwizzle::Alpha;

			texture = Texture2D::Create(specs);
		}
		
		
		m_ActiveScene = CreateRef<Scene>();
		m_EditorScene = m_ActiveScene;
		m_EditorCamera = EditorCamera(60.0f, 1.778f, 1.0f, 300.0f);
		m_PanelContext = CreateRef<PanelContext>();
		auto& commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1) {
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
		}
		else {
			NewProject();
		}

		m_PanelContext->Scene = m_ActiveScene;

		m_SceneHierarchyPanel.SetContext(m_PanelContext);
		m_PropertiesPanel.SetContext(m_PanelContext);
		//m_ConsolePanel.SetContext(::Log::GetClientLogger());
		/*m_ContentBrowserPanel.SetCurrentPath(Project::GetProjectDirectory());
		m_ContentBrowserPanel.SetStartPath(Project::GetProjectDirectory());
		m_ContentBrowserPanel.SetContext(m_PanelContext);*/

		{
			uint32_t i = 0;
			for (; i < m_OutputTextures.GetResources().size(); ++i) {
				DescriptorSetLayoutSpecification specs{};
				specs.Types = { {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} };
				m_OutputDescriptorSet.GetResources()[i] = DescriptorSet::Create(specs);
				m_OutputDescriptorSet.GetResources()[i]->Update(m_OutputTextures[i], m_OutputSampler, ImageLayout::ShaderReadOnlyOptimal, 0);
			}
		}

	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		// Resize
		HandleViewportResize();
		// Update
		if (m_SceneState == SceneState::Edit)
			m_EditorCamera.OnUpdate(ts);

		// Render
		{
			//RenderCommand::BeginRenderPass(m_OutputBuffer, m_OutputBuffer->GetDefaultRenderPass());
			//RenderCommand::EndRenderPass();
		}
		
		// Update scene
		switch (m_SceneState)
		{
		case SceneState::Edit:
		{
			//m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera, *m_OutputTextures);
			SceneRenderer renderer(m_ActiveScene.Get());
			SceneData data{};
			data.Proj = m_EditorCamera.GetProjection();
			data.View = m_EditorCamera.GetViewMatrix();
			data.ViewProj = m_EditorCamera.GetViewProjection();
			data.zNear = 0.1f;
			data.zFar = 1000.0f;
			data.ScreenSize = { m_ViewportSize.x,m_ViewportSize.y };
			data.CameraPos = m_EditorCamera.GetPosition();
			renderer.Render(*m_OutputTextures,data);
			// Project Auto Save
			auto& currentProjectConfig = Project::GetActive()->GetConfig();
			if (currentProjectConfig.ProjectAutoSave) {
				currentProjectConfig.TimeSinceLastProjectAutoSave += ts;
				if (currentProjectConfig.TimeSinceLastProjectAutoSave >= currentProjectConfig.ProjectAutoSaveTimer) {
					SaveProject();
					currentProjectConfig.TimeSinceLastProjectAutoSave = 0.0f;
				}
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
	}

	void EditorLayer::OnImGuiRender()
	{
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

			style.WindowMinSize.x = minWinSizeX;



			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					// Disabling fullscreen would allow the window to be moved to the front of other windows, 
					// which we can't undo at the moment without finer window depth/z control.
					//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);1
					if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
						m_ActiveScene->DuplicateEntity(m_PanelContext->SelectedEntity());
					if (ImGui::MenuItem("New", "Ctrl+N"))
						NewScene();

					if (ImGui::MenuItem("Open...", "Ctrl+O"))
						OpenScene();

					if (ImGui::MenuItem("Save", "Ctrl+S"))
						SaveScene();

					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
						SaveSceneAs();

					if (ImGui::MenuItem("Save Project")) {
						SaveProject();
					}


					if (ImGui::MenuItem("Import Asset...")) {
						ImportAsset();
					}


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
			//m_ConsolePanel.OnImGuiRender();
			//m_ContentBrowserPanel.OnImGuiRender();
			//m_AnimationPanel.OnImGuiRender();
			m_PropertiesPanel.OnImGuiRender();

			ShowDebugWindow();
			ShowViewport();
			UI_Toolbar();
			ImGui::End();
			//if (m_ConsoleOpen) {
			//	ImGui::Begin("Debug Console", &m_ConsoleOpen, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNavFocus);
			//	static bool core = true;
			//	static bool client = true;
			//	ImGui::Checkbox("Core", &core);
			//	ImGui::Checkbox("Client", &client);
			//	if (core) {
			//		for (auto& message : ::Log::GetCoreLogger()->GetMessages()) {

			//			ImVec4 messageColor{ 1,1,1,1 };
			//			switch (message.Level)

			//			{

			//			case MessageLevel::Info:
			//			{
			//				messageColor = { .24f,.71f,.78f,1.0f };
			//			}
			//			break;
			//			case MessageLevel::Warn:
			//			{
			//				messageColor = { .79f,.78f,.32f,1.0f };
			//			}
			//			break;
			//			case MessageLevel::Error:
			//			{
			//				messageColor = { .65f,.31f,.29f,1.0f };
			//			}
			//			break;
			//			default:
			//				break;
			//			}
			//			std::time_t time = std::chrono::system_clock::to_time_t(message.Time);
			//			std::tm tm = *std::localtime(&time);
			//			char buf[80] = { 0 };
			//			std::strftime(buf, sizeof(buf), "%d/%m/%Y-%H:%M:%S", &tm);
			//			ImGui::TextColored(messageColor, "Engine [%s] : %s", buf, message.Text.c_str());
			//		}
			//	}
			//	if (client) {
			//		for (auto& message : ::Log::GetClientLogger()->GetMessages()) {

			//			ImVec4 messageColor{ 1,1,1,1 };
			//			switch (message.Level)

			//			{

			//			case MessageLevel::Info:
			//			{
			//				messageColor = { .24f,.71f,.78f,1.0f };
			//			}
			//			break;
			//			case MessageLevel::Warn:
			//			{
			//				messageColor = { .79f,.78f,.32f,1.0f };
			//			}
			//			break;
			//			case MessageLevel::Error:
			//			{
			//				messageColor = { .65f,.31f,.29f,1.0f };
			//			}
			//			break;
			//			default:
			//				break;

			//			}
			//			std::time_t time = std::chrono::system_clock::to_time_t(message.Time);
			//			std::tm tm = *std::localtime(&time);
			//			char buf[80] = { 0 };
			//			std::strftime(buf, sizeof(buf), "%d/%m/%Y-%H:%M:%S", &tm);
			//			ImGui::TextColored(messageColor, "Editor [%s] : %s", buf, message.Text.c_str());
			//		}
			//	}
			//	ImGui::End();
			//}
		}
	}
	void EditorLayer::OnScenePlay() {
		if (!m_EditorScene || !m_ActiveScene->GetPrimaryCameraEntity())
			return;
		m_SceneState = SceneState::Play;
		m_RuntimeScene = Scene::Copy(m_EditorScene);
		m_RuntimeScene->OnRuntimeStart();
		m_ActiveScene = m_RuntimeScene;
		m_PanelContext->Scene = m_ActiveScene;
	}
	void EditorLayer::OnSceneStop() {
		m_SceneState = SceneState::Edit;
		m_ActiveScene->OnRuntimeStop();
		m_RuntimeScene = nullptr;
		m_ActiveScene = m_EditorScene;
		m_PanelContext->Scene = m_ActiveScene;
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
		m_PanelContext->Scene = m_ActiveScene;
	}
	void EditorLayer::OnSceneSimulateStop() {
		m_SceneState = SceneState::Edit;
		m_SimulationScene->OnSimulationStop();
		m_SimulationScene = nullptr;
		m_ActiveScene = m_EditorScene;
		m_ActiveScene = m_SimulationScene;
	}


	EditorLayer::GizmoCamera EditorLayer::GetCurrentCameraViewProjection()
	{
		if (m_SceneState == SceneState::Edit)
			return { m_EditorCamera.GetViewMatrix(),m_EditorCamera.GetProjection() };
		else if (m_SceneState == SceneState::Play)
		{
			auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			return { glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform()),camera.GetProjection() };
		}
	}

	void EditorLayer::MoveChildren(Entity curr, const glm::vec3& deltaTranslation, const glm::vec3& deltaRotation, Entity parent) {
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
			if (parent)
				tc.Rotation += deltaRotation;
		}
		tc.Translation += deltaTranslation;
	}

	void EditorLayer::DrawGizmos()
	{

		Entity selectedEntity = m_PanelContext->SelectedEntity();
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
				MoveEntity(selectedEntity, m_ActiveScene.Get(), deltaTranslation, deltaRotation);
				tc.Scale = scale;
			}
		}
	}

	void EditorLayer::DrawSelectedEntityOutline(Entity selectedEntity) {
		
	}

	static void GetSegmentCount(float totalSegmentCount, float* lineCount, float* segmentPerLineCount) {
		/*float maxTessLevel = RenderCommand::QueryMaxTessellationLevel();
		for (float i = 1.0f; i < maxTessLevel; i += 1.0f) {
			for (float j = 1.0f; j < maxTessLevel; j += 1.0f) {
				if (i * j >= totalSegmentCount) {
					*lineCount = i;
					*segmentPerLineCount = j;
					return;
				}
			}
		}
		*lineCount = maxTessLevel;
		*segmentPerLineCount = maxTessLevel;*/
	}

	void EditorLayer::ShowDebugWindow()
	{
		ImGui::Begin("Styler");
		ImGui::ShowStyleEditor();
		ImGui::End();
		ImGui::Begin("Stats");

		ImGui::Text("Gizmo Mode : %d", m_GizmoType);
		//auto stats = Renderer2D::GetStats();
		ImGui::Text("Frame Rate: %.3f", ImGui::GetIO().Framerate);
		for (const auto& [name, data] : AccumulativeTimer::GetTimers()) {
			float ns = data;
			float ms = (float)ns * 1e-6;
			float s = (float)ns * 1e-9;
			ImGui::TextWrapped("%s Took :(%.3f ns,%.3f ms,%.3f s)", name.c_str(), ns, ms, s);
		}

		if (ImGui::Button("x2")) {
			samples = TextureSamples::x2;
		}
		if (ImGui::Button("x4")) {
			samples = TextureSamples::x4;
		}
		if (ImGui::Button("x8")) {
			samples = TextureSamples::x8;
		}
		if (ImGui::Button("x16")) {
			samples = TextureSamples::x16;
		}

		ImGui::Text("Current samples: %d", (int)samples);


		ImGui::Text("UI Vertex Count: %d", ImGui::GetIO().MetricsRenderVertices);
		//ImGui::Image((ImTextureID)SpotLight::GetDepthMaps()->GetView(0)->GetRendererID(), {64,64}, {0,1}, {1,0});
		AccumulativeTimer::ResetTimers();


		static bool isOpen = false;
		ImGui::DragFloat3("Pos", &pos.x, 0.1);
		glm::vec3 degRot = glm::degrees(rot);
		ImGui::DragFloat3("Rot", &degRot.x, 0.1);
		rot = glm::radians(degRot);
		ImGui::DragFloat3("Scale", &scale.x, 0.1);

		ImGui::Checkbox("Open", &isOpen);

		ImGui::End();

		{
			bool shouldBeOpen = isOpen;

			if (shouldBeOpen) {

				if (ImGui::Begin("Hello", &shouldBeOpen)) {

				}

				ImGui::End();
			}

			isOpen = shouldBeOpen;

		}
		
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

		glm::vec4 uvs = _GetUVs();
		Ref<DescriptorSet> ds = *m_OutputDescriptorSet;
		ImGui::Image(reinterpret_cast<ImTextureID>(ds->GetSetID()), ImVec2{m_ViewportSize.x,m_ViewportSize.y}, {uvs.x,uvs.y}, {uvs.z,uvs.w});
		
		//ImGui::Text("Hello");
		
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

	void EditorLayer::HandleViewportResize() {
		const TextureSpecification& outputSpec = m_OutputTextures->Get()->GetTextureSpecification();
		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(outputSpec.Width != m_ViewportSize.x || outputSpec.Height != m_ViewportSize.y))
		{
			RenderCommand::DeviceWaitIdle();
			{
				Texture2DSpecification specs{};
				specs.Width = m_ViewportSize.x;
				specs.Height = m_ViewportSize.y;
				specs.Depth = outputSpec.Depth;
				specs.Layers = outputSpec.Layers;
				specs.Mips = outputSpec.Mips;
				specs.Layout = ImageLayout::General;
				specs.Samples = outputSpec.Samples;
				specs.Format = outputSpec.Format;
				specs.Swizzles[0] = TextureSwizzle::Red;
				specs.Swizzles[1] = TextureSwizzle::Green;
				specs.Swizzles[2] = TextureSwizzle::Blue;
				specs.Swizzles[3] = TextureSwizzle::Alpha;

				for (auto& output : m_OutputTextures) {
					output = Texture2D::Create(specs);
				}
			}

			{
				uint32_t i = 0;
				for (; i < m_OutputTextures.GetResources().size(); ++i) {
					DescriptorSetLayoutSpecification specs{};
					specs.Types = { {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} };
					m_OutputDescriptorSet.GetResources()[i] = DescriptorSet::Create(specs);
					m_OutputDescriptorSet.GetResources()[i]->Update(m_OutputTextures[i], m_OutputSampler, ImageLayout::ShaderReadOnlyOptimal, 0);
				}
			}
			
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}
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
			EditorIcon icon = m_Icons.IconPlay;
			if (m_SceneState == SceneState::Play)
				icon = m_Icons.IconStop;
			/*if (ImGui::ImageButton((ImTextureID)icon.GetDescriptorSet()->GetSetID(), {size,size}, {0,0}, {1,1}, 0)) {
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
			}*/
		}
		{
			if (m_SceneState == SceneState::Play) {
				ImGui::SameLine();
				EditorIcon icon = m_Icons.IconPause;
				/*if (ImGui::ImageButton((ImTextureID)icon.GetDescriptorSet()->GetSetID(), { size,size }, { 0,0 }, { 1,1 }, 0)){
					m_ActiveScene->ChangePauseState();

				}*/
			}
		}
		ImGui::SameLine();
		{
			EditorIcon icon = m_Icons.IconSimulateStart;
			if (m_SceneState == SceneState::Simulate)
				icon = m_Icons.IconSimulateStop;
			//if (ImGui::ImageButton((ImTextureID)icon.GetDescriptorSet()->GetSetID(), { size,size }, { 0,0 }, { 1,1 }, 0)) {
			//	if (m_SceneState == SceneState::Simulate)
			//		OnSceneSimulateStop();
			//	else
			//		OnSceneSimulateStart();
			//}
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
		dispatcher.Dispatch<RendererSettingsChangedEvent>(KD_BIND_EVENT_FN(EditorLayer::OnRendererSettingsChanged));
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
					m_ActiveScene->DuplicateEntity(m_PanelContext->SelectedEntity());
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
			/*if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyDown(Key::LeftAlt)) {
				m_Framebuffer->Bind();
				int pixel = GetCurrentPixelData(m_ViewportBounds, m_Framebuffer);
				m_SceneHierarchyPanel.SetSelectedEntity(pixel == -1 ? Entity{} : Entity{(entt::entity) pixel,m_ActiveScene.get() });
				m_Framebuffer->Unbind();
			}*/
		}
		return false;
	}

	bool EditorLayer::OnRendererSettingsChanged(RendererSettingsChangedEvent& e) {
		auto& settings = RendererAPI::GetSettings();

		/*if (settings.AntiAiliasing.Changed()) {
			AntiAiliasingType oldType = settings.AntiAiliasing.GetLast();
			AntiAiliasingType newType = settings.AntiAiliasing.Get();
		}*/

		if (settings.MSAASampleCount != m_RendererSettings.MSAASampleCount) {
			uint32_t oldVal = m_RendererSettings.MSAASampleCount;
			uint32_t newVal = settings.MSAASampleCount;

			/*if (oldVal != newVal) {
				KD_CORE_ASSERT(oldVal == m_OutputBuffer->GetSpecification().Samples);
				m_OutputBuffer->Resample(newVal);
				m_RendererSettings.MSAASampleCount = newVal;
			}*/

		}

		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_PanelContext->Scene = m_ActiveScene;
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

			m_PanelContext->Scene = m_EditorScene;
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

	void EditorLayer::ImportAsset() {
		auto filePath = FileDialogs::OpenFile("Texture Format (*.png)\0*.png\0");
		if (filePath) {
		}
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		try {
			if (Project::Load(path)) {
				auto startScenePath = Project::GetAbsoluteAssetPath(Project::GetActive()->GetConfig().StartScene);
				OpenScene(startScenePath);
				m_EditorScene->SetPath(startScenePath.string());
			}
		}
		catch (...) {
			NewProject();
			NewScene();
		}
		
	}

	void EditorLayer::SaveProject()
	{
		Project::SaveActive(Project::GetProjectDirectory());
	}

}
