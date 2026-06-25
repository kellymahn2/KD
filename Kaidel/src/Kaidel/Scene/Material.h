#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/DirtyCounter.h"
#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/VisualMaterial/VisualMaterial.h"
#include "Kaidel/Asset/Asset.h"

namespace Kaidel {
	class MaterialInstance : public AssetBase
	{
	public:
		virtual void Recreate() = 0;
		virtual void BindDescriptors() = 0;
		virtual void BindInstanceData(Ref<DescriptorSet> instanceDataSet) = 0;
		virtual void BindSceneData(Ref<DescriptorSet> sceneDataSet) = 0;

		virtual Ref<Shader> GetShader() = 0;

		virtual void BindInstanceOffset(uint32_t instanceOffset) = 0;

		virtual void BindPipeline() const = 0;
	};

	

	class StandardMaterialInstance : public MaterialInstance
	{
	public:
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

		struct Initializer {
			glm::vec4 BaseColor = glm::vec4(1.0f);
			float Roughness = 1.0f;
			float Metalness = 0.0f;

			MaterialTextureChannel MetalnessChannel = MaterialTextureChannel::Blue;
			MaterialTextureChannel RoughnessChannel = MaterialTextureChannel::Green;

			Ref<Texture2D> Textures[(uint32_t)MaterialTextureType::Max] = { nullptr };
		};

		virtual bool IsLoaded() const override;

	public:

		StandardMaterialInstance(const Initializer& initializer);
		
		DECL_ASSET_TYPE(StandardMaterial)

		virtual void BindDescriptors() override;


		virtual void BindInstanceData(Ref<DescriptorSet> instanceDataSet) override;


		virtual void BindInstanceOffset(uint32_t instanceOffset) override;


		virtual void BindPipeline() const override;


		virtual void BindSceneData(Ref<DescriptorSet> sceneDataSet) override;


		virtual Ref<Shader> GetShader() override;


		virtual void Recreate() override;

		MaterialUniformData& GetUniformData() { return m_UniformData; }

		void SetBaseColor(const glm::vec4& color)
		{
			m_UniformData.BaseColor = color;
		}

		const glm::vec4& GetBaseColor() const
		{
			return m_UniformData.BaseColor;
		}

		void SetTexture(MaterialTextureType type, Ref<Texture2D> image);

		void SetMetalnessColorChannel(MaterialTextureChannel channel)
		{
			m_UniformData.MetalnessChannel = channel;
			UploadUniforms();
		}

		void SetRoughnessColorChannel(MaterialTextureChannel channel)
		{
			m_UniformData.RoughnessChannel = channel;
			UploadUniforms();
		}

		void SetMetalness(float metalness)
		{
			m_UniformData.Metalness = metalness;
			UploadUniforms();
		}

		void SetRoughness(float roughness)
		{
			m_UniformData.Roughness = roughness;
			UploadUniforms();
		}

		Kaidel::Ref<Kaidel::Texture2D> GetTexture(MaterialTextureType type)
		{
			return m_Textures[(uint32_t)type];
		}

		bool IsTextureUsed(MaterialTextureType type) const
		{
			return m_UniformData.HasTexture[(uint32_t)type];
		}

		void SetTextureUsed(MaterialTextureType type, bool isUsed)
		{
			m_UniformData.HasTexture[(uint32_t)type] = isUsed;
			UploadUniforms();
		}

		void UploadUniforms()
		{
			RenderCommand::DeviceWaitIdle();
			m_UniformBuffer->SetData(&m_UniformData, sizeof(MaterialUniformData));
		}

	private:
		MaterialUniformData m_UniformData;
		Ref<UniformBuffer> m_UniformBuffer;
		Ref<DescriptorSet> m_UniformSet;

		Ref<Texture2D> m_Textures[(uint32_t)MaterialTextureType::Max] = {};
		Ref<DescriptorSet> m_TextureSet;
	};

	class VisualMaterialInstance : public MaterialInstance
	{
	public:
		VisualMaterialInstance(Ref<VisualMaterial> visualMaterial);

		virtual void BindDescriptors() override;


		virtual void BindInstanceData(Ref<DescriptorSet> instanceDataSet) override;

		virtual void BindInstanceOffset(uint32_t instanceOffset) override;


		virtual void BindPipeline() const override;

		void SetVisualMaterial(Ref<VisualMaterial> visualMaterial);

		Ref<VisualMaterial> GetVisualMaterial() const { return m_VisualMaterial; }

		virtual void BindSceneData(Ref<DescriptorSet> sceneDataSet) override;


		virtual Ref<Shader> GetShader() override;


		virtual void Recreate() override;

		void UploadUniformData();

		char* GetUniformData() { return m_UniformData.data(); }

		Ref<Texture> GetTexture(uint32_t set, uint32_t binding)
		{
			return m_Sets[set]->GetTextureAtBinding(binding);
		}

		void SetTexture(Ref<Texture> texture, uint32_t set, uint32_t binding);

		DECL_ASSET_TYPE(VisualMaterial)

	private:
		void CreateSets();
	private:
		Ref<VisualMaterial> m_VisualMaterial;

		std::unordered_map<uint32_t, Ref<DescriptorSet>> m_Sets;

		std::vector<char> m_UniformData;
		Ref<UniformBuffer> m_UniformBuffer;

		uint32_t m_LastVersion = -1;
	};

	/*
	
	class VisualMaterialInstance : MaterialInstance {

		VisualMaterial mat;
		//Added descriptors		
		//Added param arrays
	};

	
	*/
}
