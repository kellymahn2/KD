#include "AnimationPanel.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Kaidel {
	void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	void AnimationPanel::OnImGuiRender() {
		if (!m_SelectedAnimation)
			return;

		ImGui::Begin("Animation Maker");
		const auto& reg = m_SelectedAnimation->GetRegistry();
		entt::entity animID = m_SelectedAnimation->GetEnTTID();

		if (auto translationProperty = m_SelectedAnimation->GetPropertyMap<TranslationData>(); translationProperty) {
			//auto& propertyMap = translationProperty->FrameMap;
			//
			//bool opened = ImGui::TreeNode("Translation");

			//if (opened && translationProperty->KeyFrameCount) {
			//	uint32_t i = 0;
			//	for (auto it = propertyMap.begin(); it != propertyMap.end(); ++it) {
			//		ImGui::PushID(i);
			//		{
			//			float newTime = it->first;

			//			float minTimeConstraint = .0f;
			//			float maxTimeConstraint = .0f;

			//			//if(!it->second.IsKeyFrame){
			//			//	auto keyFrameBefore = translationProperty->GetKeyFrameBefore(it);
			//			//	auto keyFrameAfter = translationProperty->GetKeyFrameAfter(it);
			//			//	if (keyFrameBefore != propertyMap.end())
			//			//		minTimeConstraint = keyFrameBefore->first + .0001f;
			//			//	if (keyFrameAfter != propertyMap.end())
			//			//		maxTimeConstraint = keyFrameAfter->first - .0001f;
			//			//}
			//			//if (ImGui::DragFloat("##Translation", &newTime, .1f, minTimeConstraint, maxTimeConstraint)) {
			//			//	//translationProperty->ChangeFrame(it)
			//			//}


			//		}


			//		ImGui::PopID();
			//	}
			//	ImGui::TreePop();
			//}
		}

		ImGui::End();
	
	}



}

