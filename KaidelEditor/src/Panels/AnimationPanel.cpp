#include "AnimationPanel.h"
#include "Kaidel/Renderer/2D/Renderer2D.h"
#include "Kaidel/Renderer/RenderCommand.h"


#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <ImGuizmo.h>

namespace Kaidel {


	namespace Utils {
		static glm::vec3 ScreenToWorld(const glm::vec2& screenPos, float screenWidth, float screenHeight, const glm::mat4& convertionMatrix) {
			// Convert screen coordinates to NDC
			float ndcX = (2.0f * screenPos.x) / screenWidth - 1.0f;
			float ndcY = 1.0f - (2.0f * screenPos.y) / screenHeight;

			// Construct the homogeneous NDC position vector
			glm::vec4 ndcPosition(ndcX, ndcY, 0.0f, 1.0f);

			// Transform the NDC position to eye space
			glm::vec4 eyePosition = convertionMatrix * ndcPosition;

			// Divide by w to get the correct eye-space position
			eyePosition /= eyePosition.w;

			// Return the eye-space position as a 3D vector
			return glm::vec3(eyePosition);
		}
	}


	static SceneCamera sc;
	static inline constexpr uint32_t GraphHeight = 256;
	CustomRenderer2D CustomPointRenderer;

	struct ControlPointVertex :public PointVertex {
		int32_t PointIndex;
	};


	AnimationPanel::AnimationPanel() {
		FramebufferSpecification fbSpec{};
		fbSpec.Samples = 1;
		fbSpec.Width = 1280;
		fbSpec.Height = GraphHeight;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8,FramebufferTextureFormat::R32I };
		outputBuffer = Framebuffer::Create(fbSpec);

		Ref<VertexArray> vao = VertexArray::Create();
		Ref<VertexBuffer> vbo = VertexBuffer::Create(0);
		vbo->SetLayout({
			{ShaderDataType::Float3,"a_Position"},
			{ShaderDataType::Float4,"a_Color"},
			{ShaderDataType::Int,"a_PointIndex"}
			});
		vao->AddVertexBuffer(vbo);
		ShaderSpecification spec;

		std::string vsSource = R"(
		#version 460 core
		layout(location = 0) in vec3 a_Position;
		layout(location = 1) in vec4 a_Color;
		layout(location = 2) in int a_PointIndex;
		layout(std140, binding = 0) uniform Camera
		{
			mat4 u_ViewProjection;
			vec3 u_CameraPosition;
		};
		layout(location = 0) out vec4 v_Color;
		layout(location = 1) out flat int v_PointIndex;
		void main(){
			v_Color = a_Color;
			v_PointIndex = a_PointIndex;
			gl_Position = u_ViewProjection * vec4(a_Position,1.0);
		}
		)";
		spec.Definitions.push_back({ vsSource,ShaderType::VertexShader,false });
		std::string fsSource = R"(
		#version 460 core
		layout(location = 0) out vec4 o_Color;
		layout(location = 1) out int o_PointIndex;
		
		layout(location = 0) in vec4 v_Color;
		layout(location = 1) in flat int v_PointIndex;
		
		void main(){
			o_Color = v_Color;
			o_PointIndex = v_PointIndex;
		}

		)";
		spec.Definitions.push_back({ fsSource,ShaderType::FragmentShader,false });
		Ref<Shader> shader = Shader::Create(spec);
		CustomPointRenderer.VAO = vao;
		CustomPointRenderer.VBO = vbo;
		CustomPointRenderer.Shader = shader;
	}



	void AnimationPanel::OnImGuiRender() {


		if (FramebufferSpecification spec = outputBuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			outputBuffer->Resize((uint32_t)m_ViewportSize.x, GraphHeight);
			sc.SetViewportSize((uint32_t)m_ViewportSize.x, GraphHeight);
		}
		
		ImGui::Begin("Animation Graph");
		if(m_SelectedAnimation){
			if (auto translationProperty = m_SelectedAnimation->GetPropertyMap<TranslationData>(); translationProperty) {
				auto& frameStorage = translationProperty->FrameStorage;

				glm::vec3 maxPos{ 0,0,0 };

				for (auto& frame : frameStorage) {
					maxPos = glm::max(maxPos, glm::abs(frame.KeyFrameValue.TargetTranslation));
					for (auto& intermediate : frame.Intermediates) {
						maxPos = glm::max(maxPos, glm::abs(intermediate.TargetTranslation));
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
				colors[3] = 0.0f;
				outputBuffer->ClearAttachment(1, colors);

				uint64_t frameCount = translationProperty->KeyFrameCount + translationProperty->IntermdiateFrameCount;
				float aspect = m_ViewportSize.x / m_ViewportSize.y;
				beginData.CameraVP = glm::ortho(0.0f, (float)(frameStorage.size() - 1), -maxPos.x-1.0f, maxPos.x+1.0f);
				uint64_t accumSize = 0;

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

					/*for (auto& point : xStorage) {
						point.y /= maxPos.x;
					}*/
					outputBuffer->DisableColorAttachment(1);
					Renderer2D::DrawBezier(glm::scale(glm::mat4(1.0f),glm::vec3(1,1,1)), xStorage, glm::vec4{.2, .5, .8, 1.0});
					outputBuffer->EnableColorAttachment(1);
					Renderer2D::BeginCustomPoint<ControlPointVertex, 0>(CustomPointRenderer);
					for (uint32_t i = 1; i < xStorage.size() - 1; ++i) {
						RenderCommand::SetPointSize(10.0f);
						ControlPointVertex vertex{};
						vertex.Position = xStorage[i];
						vertex.Color = { .9,.2,.3,1.0 };
						vertex.PointIndex = i + accumSize;
						Renderer2D::DrawPoint<ControlPointVertex,0>(vertex);
					}
					Renderer2D::EndCustomPoint<ControlPointVertex, 0>();

					if (ImGui::IsMouseDown(ImGuiMouseButton_Left)&&m_CurrentControlPointBeingEdited - accumSize > 0 && m_CurrentControlPointBeingEdited-accumSize < xStorage.size() - 1) { 
						
						glm::mat4 convertionMatrix = glm::inverse(beginData.CameraVP);

						glm::vec2 oldFrameMousePos = m_LastFrameMousePos - m_ImagePos;

						glm::vec3 oldWorldPos = Utils::ScreenToWorld(oldFrameMousePos, m_ViewportSize.x, GraphHeight, convertionMatrix);

						glm::vec2 currentFrameMousePos = glm::vec2{ ImGui::GetMousePos().x,ImGui::GetMousePos().y } - m_ImagePos;

						glm::vec3 currentWorldPos = Utils::ScreenToWorld(currentFrameMousePos, m_ViewportSize.x, GraphHeight, convertionMatrix);

						glm::vec3 deltaPos = currentWorldPos - oldWorldPos;

						current.Intermediates[m_CurrentControlPointBeingEdited -accumSize - 1].TargetTranslation.x += deltaPos.y;
					}
					accumSize += xStorage.size();
					Renderer2D::End();
				}
				if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					int pixelData = 0;


					ImVec2 mousePos = ImGui::GetMousePos();
					ImVec2 windowPos = ImGui::GetWindowPos();
					ImVec2 relMousePosToWindow = { mousePos.x - windowPos.x,mousePos.y - windowPos.y };
					ImVec2 relMousePosToImage = { relMousePosToWindow.x - m_ImagePos.x,relMousePosToWindow.y - m_ImagePos.y };
					if(relMousePosToImage.x>=0 && relMousePosToImage.y>=0)
					outputBuffer->ReadValues(1, (uint32_t)relMousePosToImage.x, GraphHeight - (uint32_t)relMousePosToImage.y, 1, 1, &pixelData);
					m_CurrentControlPointBeingEdited = pixelData;
				}
				{
					ImVec2 size = ImGui::GetContentRegionAvail();
					m_ViewportSize.x = size.x;
					m_ViewportSize.y = size.y;
					m_ImagePos.x = ImGui::GetCursorPosX();
					m_ImagePos.y = ImGui::GetCursorPosY();
					ImTextureID textureID = reinterpret_cast<ImTextureID>(outputBuffer->GetColorAttachmentRendererID(0));
					ImGui::Image(textureID, { m_ViewportSize.x,GraphHeight}, { 0,1 }, { 1,0 });
				}
				m_LastFrameMousePos = { ImGui::GetMousePos().x,ImGui::GetMousePos().y };
			}
		}
		ImGui::End();

	}
}

