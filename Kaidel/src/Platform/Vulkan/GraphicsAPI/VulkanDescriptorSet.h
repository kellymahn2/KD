#pragma once
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Backend.h"


namespace Kaidel {
	class VulkanDescriptorSet : public DescriptorSet {
	public:
		VulkanDescriptorSet(const DescriptorSetSpecification& specs);
		VulkanDescriptorSet(const DescriptorSetLayoutSpecification& layout);
		VulkanDescriptorSet(Ref<Shader> shader, uint32_t setIndex);
		~VulkanDescriptorSet();

		virtual const DescriptorSetSpecification& GetSpecification()const override { return m_Specification; }

		virtual Ref<UniformBuffer> GetUniformBufferAtBinding(uint32_t i)const override { return m_Values[i].BufferValues.Buffer; }
		virtual Ref<StorageBuffer> GetStorageBufferAtBinding(uint32_t i)const override { return m_Values[i].BufferValues.Buffer; }
		virtual Ref<Texture> GetTextureAtBinding(uint32_t i)const override { return m_Values[i].ImageValues.Image; }
		virtual Ref<Sampler> GetSamplerAtBinding(uint32_t i)const override { return m_Values[i].ImageValues.ImageSampler; }

		virtual Ref<UniformBuffer> GetUniformBufferAtBinding(const std::string& name)const override {
			return GetUniformBufferAtBinding(GetBindingFromName(name));
		}
		virtual Ref<StorageBuffer> GetStorageBufferAtBinding(const std::string& name)const override {
			return GetStorageBufferAtBinding(GetBindingFromName(name));
		}
		virtual Ref<Texture> GetTextureAtBinding(const std::string& name)const override {
			return GetTextureAtBinding(GetBindingFromName(name));
		}
		virtual Ref<Sampler> GetSamplerAtBinding(const std::string& name)const override {
			return GetSamplerAtBinding(GetBindingFromName(name));
		}

		virtual DescriptorSet& Update(Ref<Buffer> buffer, uint32_t binding) override;
		virtual DescriptorSet& Update(Ref<Texture> image, Ref<Sampler> sampler, ImageLayout layout, uint32_t binding) override;

		virtual DescriptorSet& Update(Ref<Buffer> buffer, const std::string& name) override{
			return Update(buffer, GetBindingFromName(name));
		}
		virtual DescriptorSet& Update(Ref<Texture> image, Ref<Sampler> sampler, ImageLayout layout, const std::string& name) override {
			return Update(image, sampler, layout, GetBindingFromName(name));
		}

		virtual RendererID GetSetID()const override { return (RendererID)m_Info.Set; }

		const VulkanBackend::DescriptorSetInfo& GetSetInfo()const { return m_Info; }
	private:
		uint32_t GetBindingFromName(const std::string& name)const { return m_NamesToBindings.at(name); }

	private:
		VulkanBackend::DescriptorSetInfo m_Info;
		DescriptorSetSpecification m_Specification;
		std::vector<DescriptorValues> m_Values;
		std::unordered_map<std::string, uint32_t> m_NamesToBindings;
	};
}
