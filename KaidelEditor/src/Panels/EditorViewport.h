#pragma once

#include "Kaidel/Renderer/EditorCamera.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"
#include "Kaidel/Scene/ModelLibrary.h"

#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/Renderer3D.h"

#include "UI/UIHelper.h"

#include <imgui/imgui.h>

namespace Kaidel
{
#undef near
#undef far
	class EditorViewport {
	public:
		EditorViewport(const std::string& name, float fov, uint32_t width, uint32_t height, float near, float far)
		{
			m_ViewportName = name;
			m_Width = width;
			m_Height = height;
			m_EditorCamera = EditorCamera(fov, (width / (float)height), near, far);
			m_EditorCamera.SetDistance(3.0f);
		}

		void OnImGuiRender(const glm::mat4& model, Ref<Material> mat)
		{
			ImGui::PushID(this);

			Styler styler;
			styler.PushStyle(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			if (ImGui::BeginChild("##Viewport", ImVec2(m_Width, m_Height), ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY | ImGuiChildFlags_Border, ImGuiWindowFlags_NoTitleBar))
			{

				static float rotate = 0.0f;

				m_Width = ImGui::GetContentRegionAvail().x;
				m_Height = ImGui::GetContentRegionAvail().y;

				UpdateViewportSize();

				m_EditorCamera.OnUpdate(ImGui::GetIO().DeltaTime, false, true, false, false);

				SceneData sceneData{};
				sceneData.AspectRatio = m_EditorCamera.GetAspectRatio();
				sceneData.CameraPos = glm::vec4(m_EditorCamera.GetPosition(), 1.0f);
				sceneData.FOV = m_EditorCamera.GetFOV();
				sceneData.Proj = m_EditorCamera.GetProjection();
				sceneData.ScreenSize = glm::uvec2(m_Width, m_Height);
				sceneData.View = m_EditorCamera.GetViewMatrix();
				sceneData.ViewProj = m_EditorCamera.GetViewProjection();
				sceneData.zNear = m_EditorCamera.GetNear();
				sceneData.zFar = m_EditorCamera.GetFar();

				DirectionalLightData lightData{};
				lightData.Color = glm::vec4(1.0f);
				lightData.Direction = 
					-glm::normalize(glm::toMat4(glm::quat(glm::vec3(-glm::pi<float>() / 2.0f, 0.0f, 0.0f)))[2]);
				
				static Visibility vis;
				vis.Instances.push_back(InstanceData{ glm::mat4(1.0f) });

				Renderer3D::Begin(m_Outputs, vis, sceneData, lightData, m_ViewportName);

				Renderer3D::BeginColor(RendererGlobals::GetEnvironmentMap());

				Renderer3DRenderParams params;
				params.VB = ModelLibrary::GetBaseSphere()->GetVertexBuffer();
				params.IB = ModelLibrary::GetBaseSphere()->GetIndexBuffer();
				params.VertexCount = ModelLibrary::GetBaseSphere()->GetVertexCount();
				params.IndexCount = ModelLibrary::GetBaseSphere()->GetIndexCount();

				Renderer3D::Draw(params, mat);

				Renderer3D::EndColor();

				Renderer3D::End();

				ImGui::Image(static_cast<ImTextureID>(m_OutputSets->Get()->GetSetID()), ImVec2{(float)m_Width, (float)m_Height});
				m_ViewportFocused = ImGui::IsWindowFocused();
				m_ViewportHovered = ImGui::IsWindowHovered();

				ImGui::DragFloat("Rotate", &rotate, 0.5f);
				//ImGui::Text("%.3f, %.3f, %.3f", m_EditorCamera.GetPosition().x, m_EditorCamera.GetPosition().y, m_EditorCamera.GetPosition().z);
			}

			ImGui::EndChild();

			ImGui::PopID();
		}

		bool IsHovered()const { return m_ViewportHovered; }

	private:
		void UpdateViewportSize()
		{
			bool shouldCreate =
				!m_Outputs[0] ||
				(m_Width && m_Outputs->Get()->GetTextureSpecification().Width != m_Width) ||
				(m_Height && m_Outputs->Get()->GetTextureSpecification().Height != m_Height);

			if (shouldCreate)
			{
				Texture2DSpecification textureSpecs;
				textureSpecs.Width = m_Width;
				textureSpecs.Height = m_Height;
				textureSpecs.Format = Format::RGBA8UN;
				textureSpecs.Mipped = false;
				textureSpecs.Layout = ImageLayout::General;

				m_Outputs.Construct([&textureSpecs](uint32_t) {
					return Texture2D::Create(textureSpecs);
				});

				DescriptorSetLayoutSpecification setLayout(
					{ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} }
				);

				m_OutputSets.Construct([&setLayout, this](uint32_t i) {
					Ref<DescriptorSet> set = DescriptorSet::Create(setLayout);
					set->Update(m_Outputs[i], RendererGlobals::GetSamler(SamplerFilter::Linear, SamplerMipMapMode::Linear), ImageLayout::ShaderReadOnlyOptimal, 0);
					return set;
				});

				m_EditorCamera.SetViewportSize(m_Width, m_Height);
			}
		}

	private:
		std::string m_ViewportName;
		uint32_t m_Width, m_Height;

		PerFrameResource<Ref<Texture2D>> m_Outputs;
		PerFrameResource<Ref<DescriptorSet>> m_OutputSets;

		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false, m_ViewportHovered = false;

		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2];
	};
}
