#include "PropertiesPanel.h"
#include "EntityPropertiesPanel.h"
#include "AnimationPropertiesPanel.h"
#include "Kaidel/Scripting/ScriptEngine.h"
#include "Kaidel/Project/Project.h"
#include "UI/UIHelper.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <inttypes.h>
namespace Kaidel {

	void PropertiesPanel::OnImGuiRender() {
		ImGui::Begin("Properties");
		if (Entity selectedEntity = EditorContext::SelectedEntity(); selectedEntity)
		{
			EntityPropertiesPanel panel(selectedEntity);
			panel.OnImGuiRender();
		}
		else if (AnimationFrame* frame = EditorContext::SelectedAnimationFrame(); frame)
		{
			AnimationPropertiesPanel panel(frame, EditorContext::SelectedAnimationFrameValueType());
			panel.OnImGuiRender();
		}
		ImGui::End();
	}
}
