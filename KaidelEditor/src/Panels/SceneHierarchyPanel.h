#pragma once

#include "Kaidel/Core/Base.h"
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Scene/Entity.h"
#include "PropertiesPanel.h"

namespace Kaidel {

	class SceneHierarchyPanel
	{
	public:
		static void OnImGuiRender(Ref<Scene> scene);
	private:
		static void DrawEntityNode(Entity entity, Ref<Scene> scene);
	};

}
