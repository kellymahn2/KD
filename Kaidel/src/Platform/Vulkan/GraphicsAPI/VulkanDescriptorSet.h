#pragma once
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Backend.h"


namespace Kaidel {
	class VulkanDescriptorSet : public DescriptorSet {
	public:
		VulkanDescriptorSet(const DescriptorSetLayoutSpecification& layout);
		VulkanDescriptorSet(Ref<Shader> shader, uint32_t setIndex);
		~VulkanDescriptorSet();

		virtual const DescriptorSetSpecification& GetSpecification()const override { return m_Specification; }

		virtual Ref<UniformBuffer> GetUniformBufferAtBinding(uint32_t i, uint32_t arrayIndex = 0)const override {
			 return m_Values[i][arrayIndex].BufferValues.Buffer; 
		}
		virtual Ref<StorageBuffer> GetStorageBufferAtBinding(uint32_t i, uint32_t arrayIndex = 0)const override {
			 return m_Values[i][arrayIndex].BufferValues.Buffer; 
		}
		virtual Ref<Texture> GetTextureAtBinding(uint32_t i, uint32_t arrayIndex = 0)const override {
			 return m_Values[i][arrayIndex].ImageValues.Image; 
		}
		virtual Ref<Sampler> GetSamplerAtBinding(uint32_t i, uint32_t arrayIndex = 0)const override {
			 return m_Values[i][arrayIndex].ImageValues.ImageSampler; 
		}

		virtual Ref<UniformBuffer> GetUniformBufferAtBinding(const std::string& name, uint32_t arrayIndex = 0)const override {
			return GetUniformBufferAtBinding(GetBindingFromName(name), arrayIndex);
		}
		virtual Ref<StorageBuffer> GetStorageBufferAtBinding(const std::string& name, uint32_t arrayIndex = 0)const override {
			return GetStorageBufferAtBinding(GetBindingFromName(name), arrayIndex);
		}
		virtual Ref<Texture> GetTextureAtBinding(const std::string& name, uint32_t arrayIndex = 0)const override {
			return GetTextureAtBinding(GetBindingFromName(name), arrayIndex);
		}
		virtual Ref<Sampler> GetSamplerAtBinding(const std::string& name, uint32_t arrayIndex = 0)const override {
			return GetSamplerAtBinding(GetBindingFromName(name), arrayIndex);
		}

		virtual DescriptorSet& Update(Ref<Buffer> buffer, uint32_t binding, uint32_t arrayIndex = 0) override;
		virtual DescriptorSet& Update(Ref<Texture> image, Ref<Sampler> sampler, ImageLayout layout, uint32_t binding, uint32_t arrayIndex = 0) override;

		virtual DescriptorSet& Update(Ref<Buffer> buffer, const std::string& name, uint32_t arrayIndex = 0) override{
			return Update(buffer, GetBindingFromName(name), arrayIndex);
		}
		virtual DescriptorSet& Update(Ref<Texture> image, Ref<Sampler> sampler, ImageLayout layout, const std::string& name, uint32_t arrayIndex = 0) override {
			return Update(image, sampler, layout, GetBindingFromName(name), arrayIndex);
		}


		virtual RendererID GetSetID()const override { return (RendererID)m_Info.Set; }

		const VulkanBackend::DescriptorSetInfo& GetSetInfo()const { return m_Info; }
	private:
		uint32_t GetBindingFromName(const std::string& name)const { return m_NamesToBindings.at(name); }

	private:
		VulkanBackend::DescriptorSetInfo m_Info;
		DescriptorSetSpecification m_Specification;
		std::vector<std::vector<DescriptorValues>> m_Values;
		std::unordered_map<std::string, uint32_t> m_NamesToBindings;
		std::vector<VkDescriptorType> m_DescriptorTypes;
	};
}
