#include "AnimationPanel.h"
#include "Kaidel/Renderer/2D/Renderer2D.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Core/Input.h"
#include "Kaidel/Core/Timer.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <ImGuizmo.h>

namespace Kaidel {
	static SceneCamera sc;
	static inline constexpr uint32_t GraphSize = 1024;
	//CustomRenderer2D CustomPointRenderer;


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

		static void FindMinAndMaxTranslation(AnimationProperty<TranslationData>* translationProperty,glm::vec3& outMin,glm::vec3& outMax) {
			auto& frameStorage = translationProperty->FrameStorage;
			for (auto& frame : frameStorage) {
				outMin = glm::min(outMin, glm::abs(frame.KeyFrameValue.TargetTranslation));
				outMax = glm::max(outMax, glm::abs(frame.KeyFrameValue.TargetTranslation));
				for (auto& intermediate : frame.Intermediates) {
					outMin = glm::min(outMin, glm::abs(intermediate.TargetTranslation));
					outMax = glm::max(outMax, glm::abs(intermediate.TargetTranslation));
				}
			}
		}

		//static void RenderAnimationGraph(Renderer2DBeginData& beginData, std::vector<KeyFrame<TranslationData>>& frameStorage, uint32_t currentPointBeingEdited, const glm::vec2& imagePos, const glm::vec2& lastFrameMousePos, const glm::vec3& minPos, const glm::vec3& maxPos, float pixelSize) {

			/*int64_t accumSize = 0;
			for (uint64_t i = 0; i < frameStorage.size() - 1; ++i) {

				Renderer2D::Begin(beginData);

				KeyFrame<TranslationData>& current = frameStorage[i];
				KeyFrame<TranslationData>& next = frameStorage[i + 1];

				std::vector<glm::vec3> xStorage;
				xStorage.reserve(2 + current.Intermediates.size());


				xStorage.push_back({ 0 + (float)i,current.KeyFrameValue.TargetTranslation.x,0 });
				float inc = 1.0f / (float)(current.Intermediates.size() + 1);

				for (uint64_t j = 0; j < current.Intermediates.size(); ++j) {
					xStorage.push_back({ inc * (float)(j + 1) + (float)i,current.Intermediates[j].TargetTranslation.x,0 });
				}

				xStorage.push_back({ 1.0f + (float)i,next.KeyFrameValue.TargetTranslation.x,0 });


				for (auto& point : xStorage) {
					point.y = (point.y - minPos.x) / (maxPos.x*1.01f - minPos.x);
				}

				beginData.OutputBuffer->DisableColorAttachment(1);
				Renderer2D::DrawBezier(glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1)), xStorage, glm::vec4{ .2, .5, .8, 1.0 });
				beginData.OutputBuffer->EnableColorAttachment(1);
				Renderer2D::BeginCustomPoint<ControlPointVertex, 0>(CustomPointRenderer);
				for (uint32_t i = 1; i < xStorage.size() - 1; ++i) {
					RenderCommand::SetPointSize(20.0f);
					ControlPointVertex vertex{};
					vertex.Position = xStorage[i];
					vertex.Color = { .9,.2,.3,1.0 };
					vertex.PointIndex = i + accumSize;
					Renderer2D::DrawPoint<ControlPointVertex, 0>(vertex);
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && currentPointBeingEdited == vertex.PointIndex) {
						glm::vec2 oldFrameMousePos = lastFrameMousePos - imagePos;
						glm::vec2 currentFrameMousePos = glm::vec2{ ImGui::GetMousePos().x,ImGui::GetMousePos().y } - imagePos;

						float delta = (currentFrameMousePos.y - oldFrameMousePos.y) * pixelSize;
						if (Input::IsKeyDown(Key::LeftControl)) {
							delta /= 2.0f;
						}
						if (Input::IsKeyDown(Key::LeftShift)) {
							delta *= 2.0f;
						}

						current.Intermediates[i - 1].TargetTranslation.x -= delta;
						ImGui::BeginTooltip();
						ImGui::Text("%f", current.Intermediates[i - 1].TargetTranslation.x);
						ImGui::Text("%f", xStorage[i].y);
						ImGui::EndTooltip();
					}
				}
				Renderer2D::EndCustomPoint<ControlPointVertex, 0>();

				accumSize += xStorage.size();
				Renderer2D::End();
			}
		}*/
	}


	


	AnimationPanel::AnimationPanel() {
		/*FramebufferSpecification fbSpec{};
		fbSpec.Samples = 1;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Attachments = { TextureFormat::RGBA8,TextureFormat::R32I };
		outputBuffer = Framebuffer::Create(fbSpec);

		Ref<VertexBuffer> vbo = VertexBuffer::Create(0);
		vbo->SetLayout({
			{ShaderDataType::Float3,"a_Position"},
			{ShaderDataType::Float4,"a_Color"},
			{ShaderDataType::Int,"a_PointIndex"}
			});

		ShaderSpecification shaderSpec;

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
		shaderSpec.Definitions.push_back({ vsSource,ShaderType::VertexShader,false });
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
		shaderSpec.Definitions.push_back({ fsSource,ShaderType::FragmentShader,false });
		Ref<Shader> shader = Shader::Create(shaderSpec);

		VertexArraySpecification vertexArraySpec{};
		vertexArraySpec.VertexBuffers = { vbo };
		vertexArraySpec.UsedShader = shader;
		Ref<VertexArray> vao = VertexArray::Create(vertexArraySpec);
		
		CustomPointRenderer.VAO = vao;
		CustomPointRenderer.VBO = vbo;
		CustomPointRenderer.Shader = shader;*/
	}



	void AnimationPanel::OnImGuiRender() {
		//SCOPED_TIMER(Animation Panel);

		//if (FramebufferSpecification spec = outputBuffer->GetSpecification();
		//	m_ViewportSize.x > 0.0f && // zero sized framebuffer is invalid
		//	(spec.Width != m_ViewportSize.x))
		//{
		//	float aspect = m_ViewportSize.x / m_ViewportSize.y;
		//	float w = GraphSize * aspect;
		//	float h = GraphSize;
		//	outputBuffer->Resize((uint32_t)w, (uint32_t)h);
		//	sc.SetViewportSize((uint32_t)w, (uint32_t)h);
		//	sc.SetOrthographicSize(50);
		//}
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
		//if(ImGui::Begin("Animation Graph")&&m_SelectedAnimation){
		//	if (auto translationProperty = m_SelectedAnimation->GetPropertyMap<TranslationData>(); translationProperty) {
		//		

		//		Renderer2DBeginData beginData{};
		//		beginData.CameraVP = glm::mat4(1.0f);
		//		beginData.OutputBuffer = outputBuffer;


		//		auto& frameStorage = translationProperty->FrameStorage;
		//		if (frameStorage.size() <= 1) {
		//			ImGui::End();
		//			return;
		//		}

		//		glm::vec3 minPos{FLT_MAX,FLT_MAX,FLT_MAX};
		//		glm::vec3 maxPos{0,0,0};
		//		Utils::FindMinAndMaxTranslation(translationProperty,minPos,maxPos);



		//		float colors[4] = { 0,0,0,1 };
		//		outputBuffer->ClearAttachment(0, colors);
		//		colors[3] = 0.0f;
		//		outputBuffer->ClearAttachment(1, colors);

		//		float minHeight = -1.5f * maxPos.x - 1.0f;
		//		float maxHeight = 1.5f * maxPos.x + 1.0f;
		//		minHeight = -2.0f;
		//		maxHeight = 2.0f;
		//		beginData.CameraVP = sc.GetProjection();
		//		float pixelSize = (maxHeight - minHeight) / m_ViewportSize.y;

		//		Utils::RenderAnimationGraph(beginData, frameStorage, m_CurrentControlPointBeingEdited, m_ImagePos, m_LastFrameMousePos,minPos,maxPos, pixelSize);


		//		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		//			float pixelData = 0;
		//			ImVec2 mousePos = ImGui::GetMousePos();
		//			ImVec2 windowPos = ImGui::GetWindowPos();
		//			ImVec2 relMousePosToWindow = { mousePos.x - windowPos.x,mousePos.y - windowPos.y };
		//			ImVec2 relMousePosToImage = { relMousePosToWindow.x - m_ImagePos.x,relMousePosToWindow.y - m_ImagePos.y };
		//			if(relMousePosToImage.x>=0 && relMousePosToImage.y>=0)
		//				outputBuffer->ReadValues(1, (uint32_t)relMousePosToImage.x, m_ViewportSize.y - (uint32_t)relMousePosToImage.y, 1, 1, &pixelData);
		//			m_CurrentControlPointBeingEdited = pixelData;
		//		}
		//		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		//			m_CurrentControlPointBeingEdited = 0;
		//		}
		//		{
		//			ImVec2 size = ImGui::GetContentRegionAvail();
		//			m_ViewportSize.x = size.x;
		//			m_ViewportSize.y = size.y;
		//			m_ImagePos.x = ImGui::GetCursorPosX();
		//			m_ImagePos.y = ImGui::GetCursorPosY();
		//			ImTextureID textureID = reinterpret_cast<ImTextureID>(outputBuffer->GetColorAttachmentRendererID(0));
		//			ImGui::Image(textureID, { m_ViewportSize.x,m_ViewportSize.y}, { 0,1 }, { 1,0 });
		//		}
		//		m_LastFrameMousePos = { ImGui::GetMousePos().x,ImGui::GetMousePos().y };
		//	}
		//}
		//ImGui::End();
		//ImGui::PopStyleVar();
	}
}

