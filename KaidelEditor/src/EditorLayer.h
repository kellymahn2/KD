#pragma once

#include "Kaidel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/AnimationPanel.h"
#include "Panels/ConsolePanel.h"
#include "Kaidel/Renderer/EditorCamera.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D/Renderer2D.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/ParticleSystem/ParticleSystem.h"
namespace Kaidel {

	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		bool OnRendererSettingsChanged(RendererSettingsChangedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();


		void NewProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();


		void OnScenePlay();
		void OnSceneStop();
		void OnSceneSimulateStart();
		void OnSceneSimulateStop();

		void UI_Toolbar();
		void MoveChildren(Entity curr, const glm::vec3& deltaTranslation, const glm::vec3& deltaRotation,Entity parent = {});
		void DrawGizmos();
		void DrawSelectedEntityOutline(Entity selectedEntity);


		struct GizmoCamera {
			glm::mat4 View;
			const glm::mat4& Projection;
		};
		GizmoCamera GetCurrentCameraViewProjection();

		void ShowDebugWindow();

		void ShowViewport();
	private:


		
		Console m_DebugConsole;
		Ref<Framebuffer> m_3DOutputFramebuffer,m_2DOutputFrameBuffer,m_OutputBuffer,m_ScreenOutputbuffer;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene, m_RuntimeScene,m_SimulationScene;

		Entity ent;
		
		Entity m_HoveredEntity;

		bool m_PrimaryCamera = true;

		bool m_Debug = false;

		bool m_ConsoleOpen=false;
		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];


		int m_GizmoType = -1;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		ConsolePanel m_ConsolePanel;
		PropertiesPanel m_PropertiesPanel;

		AnimationPanel m_AnimationPanel;



		Ref<ComputeShader> m_2D3DCompositeShader;

		struct Icons {
			Ref<Texture2D> IconPlay;
			Ref<Texture2D> IconPause;
			Ref<Texture2D> IconSimulateStart;
			Ref<Texture2D> IconSimulateStop;
			Ref<Texture2D> IconStop;
		} m_Icons;


		enum class SceneState {
			Edit = 0, Play = 1 , Simulate = 2
		};



		SceneState m_SceneState=SceneState::Edit;

		Ref<PanelContext> m_PanelContext;
		Ref<ComputeShader> m_FXAAComputeShader;

		RendererSettings m_RendererSettings = RendererAPI::GetSettings();


	};

}
