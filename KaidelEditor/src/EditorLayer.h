#pragma once

#include "Kaidel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Kaidel/Renderer/EditorCamera.h"

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

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void OnScenePlay();
		void OnSceneStop();
		void OnSceneSimulateStart();
		void OnSceneSimulateStop();

		void UI_Toolbar();
		void MoveChildren(Entity curr, const glm::vec3& deltaTranslation, const glm::vec3& deltaRotation,Entity parent = {});
		void DrawGizmos();
		struct GizmoCamera {
			glm::mat4 View;
			const glm::mat4& Projection;
		};
		GizmoCamera GetCurrentCameraViewProjection();
		void OnOverlayRender();

		void ShowDebugWindow();

		void ShowViewport();
	private:
		Kaidel::OrthographicCameraController m_CameraController;



		Console m_DebugConsole;
		// Temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;
		Ref<Framebuffer> m_Framebuffer;
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene, m_RuntimeScene,m_SimulationScene;
		Entity m_SquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCamera;
		
		Entity m_HoveredEntity;

		bool m_PrimaryCamera = true;

		bool m_Debug = false;

		EditorCamera m_EditorCamera;

		Ref<Texture2D> m_CheckerboardTexture;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		int m_GizmoType = -1;

		// Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;


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
	};

}
