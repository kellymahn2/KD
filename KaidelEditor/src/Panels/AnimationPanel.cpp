#include "AnimationPanel.h"
#include "Kaidel/Renderer/2D/Renderer2D.h"
#include "Kaidel/Renderer/RenderCommand.h"


#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <ImGuizmo.h>

namespace Kaidel {

	static SceneCamera sc;
	static inline constexpr uint32_t GraphHeight = 256;

	AnimationPanel::AnimationPanel() {
		FramebufferSpecification fbSpec{};
		fbSpec.Samples = 1;
		fbSpec.Width = 1280;
		fbSpec.Height = GraphHeight;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8};
		outputBuffer = Framebuffer::Create(fbSpec);
	}

	void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

	void AnimationPanel::OnImGuiRender() {
		if (!m_SelectedAnimation)
			return;


		if (FramebufferSpecification spec = outputBuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			outputBuffer->Resize((uint32_t)m_ViewportSize.x, GraphHeight);
			sc.SetViewportSize((uint32_t)m_ViewportSize.x, GraphHeight);
		}
		ImGui::Begin("Animation Graph");
		
		if (auto translationProperty = m_SelectedAnimation->GetPropertyMap<TranslationData>(); translationProperty) {
			auto& frameStorage = translationProperty->FrameStorage;

			glm::vec3 maxPos{ 0,0,0 };

			for (auto& frame : frameStorage) {
				maxPos = glm::max(maxPos, frame.KeyFrameValue.TargetTranslation);
				for (auto& intermediate : frame.Intermediates) {
					maxPos = glm::max(maxPos, intermediate.TargetTranslation);
				}
			}

			Renderer2DBeginData beginData{};
			beginData.CameraVP = glm::mat4(1.0f);
			beginData.OutputBuffer = outputBuffer;

			if (frameStorage.size() <= 1) {
				ImGui::End();
				return;
			}

			float colors[4] = { 0,0,0,1 };
			outputBuffer->ClearAttachment(0, colors);

			uint64_t frameCount = translationProperty->KeyFrameCount + translationProperty->IntermdiateFrameCount;
			float aspect = m_ViewportSize.x / m_ViewportSize.y;
			beginData.CameraVP = glm::ortho(0.0f, (float)(frameStorage.size() - 1), -1.5f, 1.5f);
			for (uint64_t i = 0; i < frameStorage.size() - 1; ++i) {

				Renderer2D::Begin(beginData);
				KeyFrame<TranslationData>& current = frameStorage[i];
				KeyFrame<TranslationData>& next = frameStorage[i + 1];

				std::vector<glm::vec3> xStorage;
				xStorage.reserve(2 + current.Intermediates.size());


				xStorage.push_back({ 0 + (float)i,current.KeyFrameValue.TargetTranslation.x,0 });
				float inc = 1.0f / (float)(current.Intermediates.size() + 1);

				for (uint64_t j = 0; j < current.Intermediates.size(); ++j) {
					xStorage.push_back({ inc * (float)(j + 1) + (float)i,current.Intermediates[j].TargetTranslation.x,0});
				}

				xStorage.push_back({ 1.0f + (float)i,next.KeyFrameValue.TargetTranslation.x,0 });
				
				if (maxPos.x == 0.0)
					continue;
				for (auto& point : xStorage) {
					point.y /= maxPos.x;
				}

				Renderer2D::DrawBezier(glm::scale(glm::mat4(1.0f),glm::vec3(1,1,1)), xStorage, glm::vec4{.2, .5, .8, 1.0});
				for (uint32_t i = 1; i < xStorage.size() - 1; ++i) {
					RenderCommand::SetPointSize(10.0f);
					Renderer2D::DrawPoint(xStorage[i], { .9,.2,.3,1.0 });
				}


				Renderer2D::End();
			}

			{
				ImVec2 size = ImGui::GetContentRegionAvail();
				m_ViewportSize.x = size.x;
				m_ViewportSize.y = size.y;
				ImTextureID textureID = reinterpret_cast<ImTextureID>(outputBuffer->GetColorAttachmentRendererID(0));
				ImGui::Image(textureID, { m_ViewportSize.x,GraphHeight}, { 0,1 }, { 1,0 });
			}

		}

		ImGui::End();

	}



}

