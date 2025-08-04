#include "AnimationPropertiesPanel.h"
#include "UI/UIHelper.h"

#include <imgui/imgui.h>

namespace Kaidel {

	void AnimationPropertiesPanel::OnImGuiRender()
	{
		ImGui::Text("%.3f", m_SelectedFrame->Time);

		switch (m_FrameType)
		{
		case Kaidel::AnimationValueType::Position: DrawVec3Control("Position", m_SelectedFrame->Position.Target); break;
		case Kaidel::AnimationValueType::Rotation: DrawQuatControl("Rotation", m_SelectedFrame->Rotation.Target); break;
		case Kaidel::AnimationValueType::Scale: DrawVec3Control("Scale", m_SelectedFrame->Scale.Target, 1.0f);
		}
	}

}
