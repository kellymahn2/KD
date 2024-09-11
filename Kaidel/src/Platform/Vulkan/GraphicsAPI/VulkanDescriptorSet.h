#pragma once
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include "Backend.h"


namespace Kaidel {
	class VulkanDescriptorSet : public DescriptorSet {
	public:
		VulkanDescriptorSet(const DescriptorSetSpecification& specs);
		~VulkanDescriptorSet();

		virtual const DescriptorSetSpecification& GetSpecification()const override { return m_Specification; }
		virtual RendererID GetSetID()const override { return (RendererID)m_Info.Set; }

		const VulkanBackend::DescriptorSetInfo& GetSetInfo()const { return m_Info; }
	private:
		VulkanBackend::DescriptorSetInfo m_Info;
		DescriptorSetSpecification m_Specification;
	};
}
