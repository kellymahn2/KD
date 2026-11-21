#include "EditorLayer.h"

#include "Kaidel/Core/DebugUtils.h"
#include "Kaidel/Math/Math.h"
#include "Kaidel/Core/Timer.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "UI/AssetPreview.h"

#include "Kaidel/Scene/SceneSerializer.h"

#include "Kaidel/Utils/PlatformUtils.h"

#include "Kaidel/Scripting/ScriptEngine.h"
#include "Kaidel/Asset/AssetImporters/MaterialSerializer.h"
#include "Kaidel/Renderer/Text/Font.h"
#include "Kaidel/Scene/ModelLibrary.h"
#include "Panels/AssetChooser.h"

#include "yaml-cpp/yaml.h"

#include "imguizmo/ImGuizmo.h"
#include "Kaidel/Scene/SceneRenderer.h"
#include "Kaidel/Animation/Animation.h"

#include "Kaidel/VisualShader/VisualShader.h"
#include "Panels/VisualShaderNodeEditor.h"

#include "Kaidel/Asset/AssetManager.h"

#include "Kaidel/Renderer/Text/FontLibrary.h"

#include "UI/IconLibrary.h"
#include "UI/UIHelper.h"

#include <forward_list>
#include <inttypes.h>
#include <random>
#include <future>

namespace Kaidel {
	glm::vec4 _GetUVs();

	static Ref<VisualShader> mat;

	//Animations work.
	//TODO: Add the ability to actually render the animation curve.
	//TODO: Add the UI for animations.
	//TODO: Add the ability to load animations from animation files.
	//TODO: Add the ability to store/load animations into/from scene files.
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

	static bool IsAncestorOf(const FileSystem::path& ancestor, const FileSystem::path& child)
	{
		FileSystem::path a = FileSystem::canonical(ancestor);
		FileSystem::path c = FileSystem::canonical(child);

		if (a.is_relative())
			a = FileSystem::absolute(a);

		if (c.is_relative())
			c = FileSystem::absolute(c);

		if (a == c)
		{
			return true;
		}
		
		auto aIt = a.begin();
		auto cIt = c.begin();

		while (aIt != a.end() && cIt != c.end())
		{
			if (*aIt != *cIt)
				return false;
			++aIt;
			++cIt;
		}


		return aIt == a.end();
	}

	void EditorLayer::OnAttach()
	{
		IconLibrary::Init("Resources/Icons/");

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

		//m_Icons.IconPlay = EditorIcon("Resources/Icons/PlayButton.png",m_OutputSampler);
		//KD_INFO("Loaded Play Button");
		//m_Icons.IconPause = EditorIcon("Resources/Icons/PauseButton.png", m_OutputSampler);
		//KD_INFO("Loaded Pause Button");
		//m_Icons.IconSimulateStart = EditorIcon("Resources/Icons/SimulateButtonStart.png", m_OutputSampler);
		//KD_INFO("Loaded Simulation Play Button");
		//m_Icons.IconSimulateStop = EditorIcon("Resources/Icons/SimulateButtonStop.png", m_OutputSampler);
		//KD_INFO("Loaded Simulation Stop Button");
		//m_Icons.IconStop = EditorIcon("Resources/Icons/StopButton.png", m_OutputSampler);
		//KD_INFO("Loaded Stop Button");

		{
			RenderPassSpecification specs{};
			RenderPassAttachment attach = RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::General, TextureSamples::x1);
			attach.LoadOp = AttachmentLoadOp::Clear;
			specs.Colors.push_back(attach);
			m_OutputRenderPass = RenderPass::Create(specs);
		}

		for (auto& texture : m_OutputTextures) {
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


		
		m_EditorCamera = EditorCamera(60.0f, 1.778f, m_Near, m_Far);
		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		KD_INFO("Command line count {}", commandLineArgs.Count);
		if (commandLineArgs.Count > 1) {
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);

			if (!Project::GetActive())
			{
				NewProject();
			}
		}
		else {
			NewProject();
		}

		EditorContext::Init();
		//m_ConsolePanel.SetContext(::Log::GetClientLogger());
		/*
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

			m_OutputDepthDescriptors.Construct([](uint32_t) {
				DescriptorSetLayoutSpecification specs{};
				specs.Types = { {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} };
				return DescriptorSet::Create(specs);
				});
		}
		
		/*{
			Entity e = m_ActiveScene->CreateCube("Occluder");

			e.GetComponent<TransformComponent>().Translation = glm::vec3(0.0f, 3.0f, 0.0f);
		}
		{
			Entity e = m_ActiveScene->CreateCube("Occludee");

			e.GetComponent<TransformComponent>().Scale = glm::vec3(5.0f, 1.0f, 5.0f);
		}*/

		RendererGlobals::LoadEnvironmentMap("assets/skybox/brown_photostudio_02_4k.hdr");
	}

	void EditorLayer::OnDetach()
	{
		EditorContext::Shutdown();
		IconLibrary::Shutdown();
	}

	size_t FindFrameIndex(const std::vector<AnimationFrame>& frames, float time) {
		for (int index = 0; index < frames.size() - 1; ++index)
		{
			if (time < frames[index + 1].Time)
				return index;
		}
		return frames.size() - 1;
		KD_ASSERT(false);
	}

	void ApplyTrackToEntity(float time, const Scope<AnimationTrack>& track, Entity entity)
	{
		auto& tc = entity.GetComponent<TransformComponent>();

		if (track->Frames.empty())
			return;

		uint32_t frame = (uint32_t)FindFrameIndex(track->Frames, time);

		switch (track->ValueType)
		{
		case Kaidel::AnimationValueType::Position:
		{
			if (frame == track->Frames.size() - 1)
			{
				tc.Translation = track->Frames.back().Position.Target;
			}
			else
			{
				float t = (time - track->Frames[frame].Time) / (track->Frames[frame + 1].Time - track->Frames[frame].Time);
				tc.Translation = glm::mix(track->Frames[frame].Position.Target, track->Frames[frame + 1].Position.Target, t);
			}
		} break;
		case Kaidel::AnimationValueType::Rotation:
		{

			if (frame == track->Frames.size() - 1)
			{
				tc.Rotation = track->Frames.back().Rotation.Target;
			}
			else
			{
				float t = (time - track->Frames[frame].Time) / (track->Frames[frame + 1].Time - track->Frames[frame].Time);
				tc.Rotation = glm::normalize(glm::slerp(track->Frames[frame].Rotation.Target, track->Frames[frame + 1].Rotation.Target, t));
			}
		} break;
		case Kaidel::AnimationValueType::Scale:
		{
			//if (frame == track->Frames.size() - 1)
			//{
			//	tc.Scale = track->Frames.back().Scale.Target;
			//}
			//else
			//{
			//	float t = (time - track->Frames[frame].Time) / (track->Frames[frame + 1].Time - track->Frames[frame].Time);
			//	tc.Scale = glm::mix(track->Frames[frame].Scale.Target, track->Frames[frame + 1].Scale.Target, t);
			//}
		} break;
		}
	}

	void UpdateAnimationRecursive(float time, AnimationTree::AnimationTreeNode& currNode, Entity currEntity, Scene* scene)
	{
		for (uint32_t i = 0; i < AnimationValueTypeCount; ++i)
		{
			if (!currNode.Tracks[i])
				continue;

			ApplyTrackToEntity(time, currNode.Tracks[i], currEntity);
		}

		if (currNode.Children.empty())
		{
			return;
		}

		if (currEntity.HasComponent<ParentComponent>())
		{
			auto& pc = currEntity.GetComponent<ParentComponent>();

			uint32_t entitiesUpdated = 0;

			for (auto& childID : pc.Children)
			{
				Entity childEntity = { childID, scene };

				auto it = currNode.Children.find(childEntity.GetComponent<TagComponent>().Tag);

				if (it != currNode.Children.end())
				{
					UpdateAnimationRecursive(time, it->second, childEntity, scene);
					++entitiesUpdated;

					//Break early when all entities have updated.
					if (entitiesUpdated == currNode.Children.size())
					{
						break;
					}
				}
			}

			KD_CORE_ASSERT(entitiesUpdated == currNode.Children.size());
		}
	}

	void UpdateAnimation(float time, AnimationTree& tree, Entity root, Scene* scene)
	{
		UpdateAnimationRecursive(time, tree.RootNode, root, scene);
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		// Resize
		HandleViewportResize();
		// Update
		if (m_SceneState == SceneState::Edit)
			m_EditorCamera.OnUpdate(ts, m_ViewportHovered, m_ViewportHovered, m_ViewportHovered, m_ViewportHovered);

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
			SceneData data;
			data.Proj = m_EditorCamera.GetProjection();
			data.View = m_EditorCamera.GetViewMatrix();
			data.ViewProj = m_EditorCamera.GetViewProjection();
			data.zNear = m_Near;
			data.zFar = m_Far;
			data.ScreenSize = { m_ViewportSize.x,m_ViewportSize.y };
			data.CameraPos = glm::vec4(m_EditorCamera.GetPosition(), 1.0f);
			data.FOV = m_EditorCamera.GetFOV();
			data.AspectRatio = m_EditorCamera.GetAspectRatio();
			renderer.Render(m_OutputTextures, data);

			// Project Auto Save
			auto& currentProjectConfig = Project::GetActive()->GetConfig();
			if (currentProjectConfig.ProjectAutoSave && FileSystem::exists(Project::GetProjectDirectory())) {
				currentProjectConfig.TimeSinceLastProjectAutoSave += ts;
				if (currentProjectConfig.TimeSinceLastProjectAutoSave >= currentProjectConfig.ProjectAutoSaveTimer) {
					SaveProject();
					currentProjectConfig.TimeSinceLastProjectAutoSave = 0.0f;
				}
			}

			{
				auto view = m_ActiveScene->m_Registry.view<AnimationPlayerComponent>();

				for (auto e : view)
				{
					auto& [apc] = view.get(e);

					if (apc.Animation && apc.PlaybackSpeed != 0.0f && apc.State != AnimationPlayerComponent::PlayerState::Paused)
					{
						UpdateAnimation(apc.Time, *apc.Animation, Entity(e, m_ActiveScene.Get()), m_ActiveScene.Get());

						apc.Time += ts.GetSeconds() * apc.PlaybackSpeed;

						if (apc.Time >= apc.Animation->Duration)
						{
							switch (apc.FinishAction)
							{
							case AnimationPlayerComponent::AnimationOnFinishAction::None: apc.State = AnimationPlayerComponent::PlayerState::Paused; break;
							case AnimationPlayerComponent::AnimationOnFinishAction::Repeat: apc.State = AnimationPlayerComponent::PlayerState::Playing; break;
							}

							apc.Time = 0.0f;
						}
					}
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
						m_ActiveScene->DuplicateEntity(EditorContext::SelectedEntity());
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
				if (ImGui::BeginMenu("Editor"))
				{
					if (ImGui::MenuItem("Refresh asset previews"))
					{
						AssetPreview::SetDirtyAll();
					}
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}


			m_SceneHierarchyPanel.OnImGuiRender(m_ActiveScene);
			//m_ConsolePanel.OnImGuiRender();4
			ContentBrowserPanel::OnImGuiRender();
			AnimationPanel::OnImGuiRender();
			PropertiesPanel::OnImGuiRender();
			VisualShaderNodeEditor::OnImGuiRender();
			AssetChooser::OnImGuiRender();

			ShowDebugWindow();
			ShowViewport();
			//UI_Toolbar();
			ImGui::End();

			if (ImGui::Begin("Shaders"))
			{
				ImGui::Columns(2);
				std::vector<std::pair<Path, Ref<Shader>>> shaders(ShaderLibrary::GetAllShaders().begin(), ShaderLibrary::GetAllShaders().end());
				std::sort(shaders.begin(), shaders.end(), [](const auto& a1, const auto& a2) {
					return std::less<Path>()(a1.first, a2.first);
					});

				float maxWidth = 0.0f;
				for (auto& [path, shader] : shaders)
				{
					maxWidth = std::max(maxWidth, ImGui::CalcTextSize(path.string().c_str()).x);
				}
				if (!shaders.empty())
				{
					ImGui::SetColumnWidth(0, maxWidth + 20.0f);
				}

				for (auto& [path, shader] : shaders)
				{
					bool changed = ShaderLibrary::GetTimeAtLoad(path) != FileSystem::last_write_time(path).time_since_epoch().count();

					if (changed)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

					ImGui::PushID(shader.Get());
					ImGui::Text(path.string().c_str());
					ImGui::NextColumn();
					if (ImGui::Button("Recreate"))
					{
						ShaderLibrary::UpdateShader(path);
					}
					ImGui::NextColumn();
					ImGui::PopID();

					if (changed)
						ImGui::PopStyleColor();

				}
				ImGui::Columns(1);
			}
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
	}
	void EditorLayer::OnSceneStop() {
		m_SceneState = SceneState::Edit;
		m_ActiveScene->OnRuntimeStop();
		m_RuntimeScene = nullptr;
		m_ActiveScene = m_EditorScene;
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
		else
			return { glm::mat4(1.0f),glm::mat4(1.0f) };
	}

	void EditorLayer::DrawGizmos()
	{

		Entity selectedEntity = EditorContext::SelectedEntity();
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
			glm::mat4 transform = tc.GlobalTransform;

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

	void ShowTextures(const std::string& name, DescriptorSet* set, const ImVec2& size) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin(name.c_str(), nullptr);
		ImGui::Image((ImTextureID)set->GetSetID(), size);
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::ShowDebugWindow()
	{
		if (ImGui::Begin("Debug Assets"))
		{
			ImGui::Text("%" PRIu64, AssetManager::GetAssetCount());

			if (ImGui::BeginTable("##Assets", 3, ImGuiTableFlags_Resizable))
			{
				ImGui::TableSetupColumn("ID");
				ImGui::TableSetupColumn("Path");
				ImGui::TableSetupColumn("Type");
				ImGui::TableHeadersRow();

				for (auto& [id, asset] : AssetManager::BeginLoaded())
				{
					if (!asset)
					{
						continue;
					}

					const AssetMetadata& metadata = AssetManager::GetAssetMetadata(id);

					bool ready = asset->IsLoaded();

					bool dirty = metadata.Dirty;

					ImVec4 color;

					if (ready && dirty)
						color = ImVec4(0.93f, 0.10f, 0.4f, 1.0f);
					else if (ready)
						color = ImVec4(0.1f, 0.89f, 0.23f, 1.0f);
					else
						color = ImVec4(0.93f, 0.12f, 0.15f, 1.0f);
					
					AssetType type = asset->GetAssetType();

					const char* typeName = "";

					#define DECL_TYPE(type) case AssetType::##type: typeName = #type; break;

					switch (type)
					{
						DECL_TYPE(SpriteSheet)
							DECL_TYPE(Font)
							DECL_TYPE(Model)
							DECL_TYPE(Mesh)
							DECL_TYPE(StandardMaterial)
							DECL_TYPE(VisualShader)
					}

					ImGui::PushStyleColor(ImGuiCol_Text, color);

					ImGui::TableNextColumn();
					ImGui::Text("%llu", (uint64_t)id);
					
					ImGui::TableNextColumn();
					ImGui::Text(metadata.Path.string().c_str());

					ImGui::TableNextColumn();
					ImGui::Text(typeName);

					ImGui::PopStyleColor();
				}

				ImGui::EndTable();
			}

		}

		ImGui::End();
		
		ImGui::Begin("Styler");
		ImGui::ShowStyleEditor();
		ImGui::End();

		if (ImGui::Begin("Debug textures"))
		{
			for (auto& [name, texture] : DebugUtils::GetDebugTextures())
			{
				DescriptorSetLayoutSpecification specs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });
				Ref<DescriptorSet> set = DescriptorSet::Create(specs);

				if (texture->GetTextureSpecification().Layout != ImageLayout::ShaderReadOnlyOptimal)
				{
					ImageMemoryBarrier barrier(
						texture, ImageLayout::ShaderReadOnlyOptimal, Utils::LayoutToAccessFlags(texture->GetTextureSpecification().Layout),
						AccessFlags_ShaderRead
					);

					RenderCommand::PipelineBarrier(
						Utils::LayoutToPipelineStages(texture->GetTextureSpecification().Layout),
						PipelineStages_FragmentShader, {}, {}, {barrier});
				}

				set->Update(texture, GLOBAL_SAMPLER_LINEAR, ImageLayout::ShaderReadOnlyOptimal, 0);

				ImGui::Text(name.c_str());
				ImGui::SameLine();
				ImGui::Image((ImTextureID)set->GetSetID(), ImVec2(240, 240));
			}
		}

		DebugUtils::Reset();

		ImGui::End();


		ImGui::Begin("Stats");

		ImGui::Text("UI Frame Rate: %.3f", ImGui::GetIO().Framerate);
		ImGui::Text("UI Frame Time: %.3f", 1.0f / ImGui::GetIO().Framerate);

		ImGui::Text("App Frame Rate: %.3f", 1.0f / Application::Get().GetDeltaTime());
		ImGui::Text("App Frame Time: %.3f", Application::Get().GetDeltaTime());
		ImGui::Text("App Running Time: %.3f", Application::Get().GetRunningTime());



		ImGui::Text("Draw calls: %" PRIu64, Renderer3D::GetStats().DrawCalls);
		ImGui::Text("Instance count: %" PRIu64, Renderer3D::GetStats().InstanceCount);
		ImGui::Text("Index count: %" PRIu64, Renderer3D::GetStats().IndexCount);
		ImGui::Text("Vertex count: %" PRIu64, Renderer3D::GetStats().VertexCount);

		ImGui::Begin("Timers");
		static bool stopTimers = false;
		static std::unordered_map<std::string, uint64_t> stoppedTimers;
		if (ImGui::Button(stopTimers ? "Start timers" : "Stop timers"))
		{
			stopTimers = !stopTimers;
			if (stopTimers)
			{
				stoppedTimers = AccumulativeTimer::GetTimers();
			}
		}

		if (ImGui::BeginTable("##Timers", 4, ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable, ImVec2(0.0f, 200.0f)))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("ns");
			ImGui::TableSetupColumn("ms");
			ImGui::TableSetupColumn("s");
			ImGui::TableHeadersRow();
			
			for (const auto& [name, data] : stopTimers ? stoppedTimers : AccumulativeTimer::GetTimers()) {
				ImGui::TableNextColumn();

				ImGui::Text(name.c_str());

				float ns = (float)data;
				float ms = (float)ns * 1e-6f;
				float s = (float)ns * 1e-9f;

				ImGui::TableNextColumn();
				ImGui::Text("%.3f", ns);
				
				ImGui::TableNextColumn();
				ImGui::Text("%.4f", ms);
				
				ImGui::TableNextColumn();
				ImGui::Text("%.5f", s);
			}


			ImGui::EndTable();
		}

		
		ImGui::End();

		ImGui::Text("UI Vertex Count: %d", ImGui::GetIO().MetricsRenderVertices);
		
		AccumulativeTimer::ResetTimers();
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

		glm::vec4 uvs = _GetUVs();
		Ref<DescriptorSet> ds = *m_OutputDescriptorSet;
		ImGui::Image(static_cast<ImTextureID>(ds->GetSetID()), ImVec2{m_ViewportSize.x,m_ViewportSize.y});

		{
			DragDropTarget target;

			if (auto modelHandle = target.Receive<AssetHandle>(Model::GetStaticAssetTypeString()); modelHandle)
			{
				Ref<Model> model = AssetManager::GetAsset(*modelHandle.Data).As<Model>();

				auto& trans = m_ActiveScene->CreateModel(model).GetComponent<TransformComponent>();

				glm::vec2 mousePos = (glm::vec2&)ImGui::GetMousePos() - m_ViewportBounds[0];

				glm::vec4 pos = glm::inverse(m_EditorCamera.GetViewProjection()) * glm::vec4(mousePos / m_ViewportSize * 2.0f - 1.0f, 0.25f, 1.0f);

				pos /= pos.w;

				trans.Translation = pos;
			}
		}
		
		//ImGui::Text("Hello");
		
		/*if (ImGui::BeginDragDropTarget()) {
			if (auto payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(path);
			}
			ImGui::EndDragDropTarget();
		}*/
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
				specs.Width = (uint32_t)m_ViewportSize.x;
				specs.Height = (uint32_t)m_ViewportSize.y;
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
			//EditorIcon icon = m_Icons.IconPlay;
			//if (m_SceneState == SceneState::Play)
				//icon = m_Icons.IconStop;
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
				//ImGui::SameLine();
				//EditorIcon icon = m_Icons.IconPause;
				/*if (ImGui::ImageButton((ImTextureID)icon.GetDescriptorSet()->GetSetID(), { size,size }, { 0,0 }, { 1,1 }, 0)){
					m_ActiveScene->ChangePauseState();

				}*/
			}
		}
		ImGui::SameLine();
		{
			//EditorIcon icon = m_Icons.IconSimulateStart;
			//if (m_SceneState == SceneState::Simulate)
				//icon = m_Icons.IconSimulateStop;
			//if (ImGui::ImageButton((ImTextureID)icon.GetDescriptorSet()->GetSetID(), { size,size }, { 0,0 }, { 1,1 }, 0)) {
			//	if (m_SceneState == SceneState::Simulate)
			//		OnSceneSimulateStop();
			//	else
			//		OnSceneSimulateStart();
			//}
		}
		ImGui::Text("H");
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
					m_ActiveScene->DuplicateEntity(EditorContext::SelectedEntity());
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
		Ref<Scene> scene = CreateRef<Scene>();
		scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

		{
			Entity e = scene->CreateEntity("Light");
			auto& tc = e.GetComponent<TransformComponent>();
			tc.Rotation = glm::quat(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f));
			auto& dlc = e.AddComponent<DirectionalLightComponent>();
			dlc.Color = glm::vec3(1.0);
			dlc.MaxDistance = m_Far;
			dlc.SplitLambda = 0.95f;
			dlc.FadeStart = 1.0f;
		}

		scene->SetPath("Untitled.kaidel");

		Path assetPath = Project::GetActive()->GetProjectDirectory() / scene->GetPath();

		m_EditorScene = m_ActiveScene = scene;
		
		if (FileSystem::exists(assetPath))
		{
			SaveSceneAs();
		}
		else
		{
			AssetMetadata mtd;
			mtd.Path = assetPath;
			mtd.Type = AssetType::Scene;

			AssetManager::RegisterAsset(scene, mtd);
			SaveScene();
		}
	}

	bool EditorLayer::OpenScene()
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();
		std::optional<std::string> filepath = FileDialogs::OpenFile("Kaidel Scene (*.Kaidel)\0*.Kaidel\0");
		if (filepath)
		{
			return OpenScene(*filepath);
		}

		return false;
	}

	bool EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();
		if (path.extension().string() != ".Kaidel" && path.extension().string() != ".kaidel")
		{
			KD_ERROR("Could not load {0} - not a scene file", path.filename().string());
			return false;
		}
		AssetMetadata mtd;
		mtd.Path = path;
		Ref<Scene> newScene = AssetManager::LoadAsset(mtd, AssetType::Scene, {}, AssetLoadPolicy::Synchronous).As<Scene>();
		if (newScene)
		{
			newScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			newScene->SetPath(path.string());
			m_EditorScene = newScene;
			m_ActiveScene = m_EditorScene;
			return true;
		}
		
		return false;
	}

	void EditorLayer::SaveScene()
	{
		if (m_EditorScene->GetPath().empty())
		{
			SaveSceneAs();
			return;
		}

		Project::GetActive()->SaveAssets();

		AssetManager::Serialize(Project::GetActive()->GetProjectDirectory() / m_EditorScene->GetPath(), Asset(m_EditorScene), true);
	}

	void EditorLayer::SaveSceneAs()
	{
		std::optional<std::string> filepath = FileDialogs::SaveFile("Kaidel Scene (*.kaidel)\0*.kaidel\0");
		if (filepath)
		{
			AssetMetadata mtd;
			mtd.Path = *filepath;
			mtd.Type = AssetType::Scene;

			Project::GetActive()->SaveAssets();
			AssetManager::RegisterAsset(m_EditorScene, mtd);
			AssetManager::Serialize(*filepath, Asset(m_EditorScene), true);
			m_EditorScene->SetPath(FileSystem::relative(*filepath, Project::GetProjectDirectory()).string());
		}
	}

	bool EditorLayer::ImportAsset() {
		return true;
	}

	static void OnProjectFileAddedEvent(const std::string& path)
	{
		Path assetPath = Project::GetProjectDirectory() / Path(path);

		auto extension = assetPath.extension();

		const auto& supportedAssets = AssetManager::GetSupportedAssets();

		auto it = supportedAssets.find(extension.string());

		if (it != supportedAssets.end())
		{
			AssetMetadata mtd;
			mtd.Path = assetPath;

			AssetManager::LoadAsset(mtd, it->second);
		}
	}

	static void OnProjectFileModifiedEvent(const std::string& path, Ref<Scene>& activeScene)
	{
		return;
		Path assetPath = Project::GetProjectDirectory() / Path(path);

		if (!AssetManager::GetAssetAtPath(assetPath))
		{
			return;
		}
		
		auto [asset, handle] = AssetManager::RefreshAsset(assetPath);

		AssetPreview::SetDirty(handle);
	}

	void EditorLayer::NewProject()
	{
		std::string path = *FileDialogs::SelectFolder();


		Path prjPath;

		for (auto& entry : FileSystem::directory_iterator(path))
		{
			if (entry.path().extension() == ".kdprj")
			{
				prjPath = entry.path();
				break;
			}
		}

		if (!prjPath.empty() && OpenProject(prjPath))
		{
			return;
		}

		Project::New(path);

		FileWatcherHandlers handlers;
		handlers.OnAdded = &OnProjectFileAddedEvent;
		handlers.OnModified = [this](auto path) {OnProjectFileModifiedEvent(path, this->m_ActiveScene); };

		Project::GetActive()->SetFileWatcher(Project::GetProjectDirectory().string(), handlers);
		
		NewScene();
		
		SaveProject();
	}

	bool EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if(Project::GetActive())
			SaveProject();

		if (!FileSystem::exists(path) || FileSystem::is_empty(path))
		{
			return false;
		}

		try {
			if (Project::Load(path)) {
				auto startScenePath = Project::GetAbsoluteAssetPath(Project::GetActive()->GetConfig().StartScene);
				
				FileWatcherHandlers handlers;
				handlers.OnAdded = &OnProjectFileAddedEvent;
				handlers.OnModified = [this](auto path) {OnProjectFileModifiedEvent(path, this->m_ActiveScene); };

				Project::GetActive()->SetFileWatcher(Project::GetProjectDirectory().string(), handlers);

				bool result = false;

				if (FileSystem::exists(startScenePath))
				{
					result = OpenScene(startScenePath);
				}
				else
				{
					NewScene();

					result = true;
				}

				Application::Get().ExecuteMainThreadQueue();

				return result;
			}
		}
		catch (std::exception& error) {
			KD_ERROR("Couldn't load project at path: {} \n {}", path.string(), error.what());
		}

		return false;
	}

	void EditorLayer::SaveProject()
	{
		if (!FileSystem::exists(Project::GetProjectDirectory()))
		{
			//FileDialogs::OpenFile()
		}

		if(AssetManager::GetAssetMetadata(m_EditorScene->GetHandle()).Dirty)
			SaveScene();

		Project::GetActive()->GetConfig().StartScene = m_EditorScene->GetPath();
		Project::SaveActive(Project::GetProjectDirectory());
	}

}
