#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Kaidel/Renderer/RendererGlobals.h"

namespace Kaidel {
	enum class MaterialTextureType {
		Albedo = 1,
		Normal,
		Mettalic,
		Roughness,
		Max
	};


	class Material : public IRCCounter<false> {
	public:
		Material(Ref<RenderPass> renderPass) {

			if (!s_Pipeline) {
				{
					GraphicsPipelineSpecification specs;

					specs.Input.Bindings.push_back(VertexInputBinding({
						{"a_Position",Format::RGB32F},
						{"a_TexCoords",Format::RG32F},
						{"a_Normal",Format::RGB32F},
						{"a_Tangent",Format::RGB32F},
						{"a_BiTangent",Format::RGB32F},
						}));
					specs.Multisample.Samples = TextureSamples::x1;
					specs.Primitive = PrimitiveTopology::TriangleList;
					specs.Rasterization.FrontCCW = true;
					specs.Rasterization.CullMode = PipelineCullMode::None;
					specs.Shader = ShaderLibrary::LoadShader("DeferredGBufferGen", "assets/_shaders/DeferredGBufferGen.shader");
					specs.RenderPass = renderPass;
					specs.Subpass = 0;
					specs.DepthStencil.DepthTest = true;
					specs.DepthStencil.DepthWrite = true;
					specs.DepthStencil.DepthCompareOperator = CompareOp::Less;
					s_Pipeline = GraphicsPipeline::Create(specs);
				}
			}

			m_Pipeline = s_Pipeline;
			m_TextureSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0);

			Ref<Texture> defaultWhite = RendererGlobals::GetSingleColorTexture(glm::vec4(1.0f));

			for (uint32_t i = 1; i < (uint32_t)MaterialTextureType::Max; ++i) {
				m_TextureSet->Update(defaultWhite, {}, ImageLayout::ShaderReadOnlyOptimal, i);
			}

			Ref<Sampler> sampler = RendererGlobals::GetSamler(SamplerFilter::Linear, SamplerMipMapMode::Linear);

			m_TextureSet->Update({}, sampler, {}, (uint32_t)0);
		}

		~Material() = default;

		void BindValues() {
			RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), m_TextureSet, 0);
		}


		void BindBaseValues(const glm::mat4& viewProj) {
			RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0, viewProj);
		}

		void BindTransform(const glm::mat4& transform) {
			RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), sizeof(glm::mat4), transform);
		}

		void BindPipeline()const {
			RenderCommand::BindGraphicsPipeline(s_Pipeline);
		}

		Ref<GraphicsPipeline> GetPipeline()const { return s_Pipeline; }

		Ref<DescriptorSet> GetTextureSet()const { return m_TextureSet; }

		const glm::vec3& GetAlbedoColor()const { return m_Values.AlbedoColor; }
		void SetAlbedoColor(const glm::vec3& color) { m_Values.AlbedoColor = color; }

		const glm::vec3& GetSpecular()const { return m_Values.Specular; }
		void SetSpecular(const glm::vec3& value) { m_Values.Specular = value; }

		const glm::vec3& GetMetallic()const { return m_Values.Metallic; }
		void SetMetallic(const glm::vec3& value) { m_Values.Metallic = value; }

		const glm::vec3& GetRoughness()const { return m_Values.Roughness; }
		void SetRoughness(const glm::vec3& value) { m_Values.Roughness = value; }

		void SetTexture(MaterialTextureType type, Ref<Texture2D> image) {
			m_Textures[(uint32_t)type] = image;
			if (image)
				m_TextureSet->Update(image, {}, ImageLayout::ShaderReadOnlyOptimal, (uint32_t)type);
		}
	private:
		struct MaterialValues {
			glm::vec3 AlbedoColor = {};
			glm::vec3 Specular = {};
			glm::vec3 Metallic = {};
			glm::vec3 Roughness = {};
		};
	private:

		MaterialValues m_Values;

		Ref<UniformBuffer> m_MaterialData;
		Ref<DescriptorSet> m_DataSet;

		Ref<GraphicsPipeline> m_Pipeline;

		Ref<Texture2D> m_Textures[(uint32_t)MaterialTextureType::Max] = {};
		Ref<DescriptorSet> m_TextureSet;

		static inline Ref<RenderPass> s_RenderPass;

		static inline Ref<GraphicsPipeline> s_Pipeline;
	};
}
