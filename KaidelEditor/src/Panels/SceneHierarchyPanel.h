#pragma once

#include "Kaidel/Core/Base.h"
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Scene/Entity.h"
#include "PropertiesPanel.h"

namespace Kaidel {

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const Ref<Scene>& scene);

		void SetContext(Ref<PanelContext> context) {
			m_Context = context;
		}

		void OnImGuiRender();
		void RegisterFieldRenderers();
	private:
		void DrawEntityNode(Entity entity);
	private:
		Ref<PanelContext> m_Context;
	};

}
