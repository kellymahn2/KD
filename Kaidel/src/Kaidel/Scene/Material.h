#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/DirtyCounter.h"
#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Kaidel/Renderer/RendererGlobals.h"

namespace Kaidel {
	enum class MaterialTextureType {
		Albedo = 0,
		Normal,
		Metallic,
		Roughness,
		Emissive,
		Max
	};


	enum class MaterialTextureChannel : uint32_t {
		Red, Green, Blue
	};

	struct MaterialUniformData
	{
		glm::vec4 BaseColor = glm::vec4(1.0f);
		float Roughness = 1.0f;
		float Metalness = 0.0f;

		MaterialTextureChannel MetalnessChannel = MaterialTextureChannel::Blue;
		MaterialTextureChannel RoughnessChannel = MaterialTextureChannel::Green;
		
		uint32_t HasTexture[(uint32_t)MaterialTextureType::Max] = { 0 };
	};

	class Material : public IRCCounter<false> {
	public:
		static Ref<Material> CreateFrom(Ref<Material> src);

		Material() = default;

		Material(Ref<RenderPass> renderPass);

		~Material() = default;

		void BindValues();

		void BindInstanceData(Ref<DescriptorSet> instanceDataSet);

		void BindSceneData(Ref<DescriptorSet> sceneDataSet);

		Ref<Shader> GetShader();

		void BindTransform(const glm::mat4& transform);

		void BindPipeline()const;

		Ref<GraphicsPipeline> GetPipeline()const;

		Ref<DescriptorSet> GetTextureSet()const;

		const MaterialUniformData& GetUniformData() const;

		void SetBaseColor(const glm::vec4& color);

		const glm::vec4& GetBaseColor() const;

		void SetTexture(MaterialTextureType type, Ref<Texture2D> image);

		void SetMetalnessColorChannel(MaterialTextureChannel channel);

		void SetRoughnessColorChannel(MaterialTextureChannel channel);

		void SetMetalness(float metalness);

		void SetRoughness(float roughness);

		Ref<Texture2D> GetTexture(MaterialTextureType type);

		bool IsTextureUsed(MaterialTextureType type);

		void SetTextureUsed(MaterialTextureType type, bool isUsed);

	private:
		void UploadUniforms();

	private:
		MaterialUniformData m_UniformData;

		Ref<UniformBuffer> m_MaterialBuffer;
		Ref<DescriptorSet> m_DataSet;

		Ref<GraphicsPipeline> m_Pipeline;

		Ref<Texture2D> m_Textures[(uint32_t)MaterialTextureType::Max] = {};
		Ref<DescriptorSet> m_TextureSet;
		bool m_IsDirty = false;

		static inline Ref<RenderPass> s_RenderPass;

		static inline Ref<GraphicsPipeline> s_Pipeline;
	};
}
