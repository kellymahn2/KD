#pragma once


#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"
#include "Platform/Vulkan/VulkanDefinitions.h"


namespace Kaidel {

	class VulkanGraphicsPipeline : public GraphicsPipeline {
	public:
		VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specification);
		~VulkanGraphicsPipeline();

		GraphicsPipelineSpecification& GetSpecification() override { return m_Specification; }
		void Recreate() override;
		RendererID GetRendererID() const override { return (RendererID)m_Pipeline; }

		VkPipelineLayout GetLayout()const { return m_Layout; }
		VkDescriptorSetLayout GetSetLayout(uint32_t index)const { return m_SetLayouts[index]; }
	private:
		void DestroyPipeline();
	private:
		GraphicsPipelineSpecification m_Specification;
		VkPipeline m_Pipeline;
		std::vector<VkDescriptorSetLayout> m_SetLayouts;
		VkPipelineLayout m_Layout;
	};

}
