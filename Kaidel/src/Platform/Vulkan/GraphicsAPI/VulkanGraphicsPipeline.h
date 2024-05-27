#pragma once

#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"

#include "PerFrameResource.h"


namespace Kaidel {
	namespace Vulkan {

		class VulkanGraphicsPipeline : public GraphicsPipeline {
		public:
			VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specification);
			~VulkanGraphicsPipeline();
			
			void Finalize() override;
			
			VkPipeline GetPipeline()const { return m_Pipeline; }
			VkPipelineLayout GetLayout()const { return m_Layout; }
			VkDescriptorSet GetDescriptorSet()const { return *m_UniformBufferDescriptorSets; }

		private:
			GraphicsPipelineSpecification m_Specification;
			VkPipeline m_Pipeline = VK_NULL_HANDLE;
			VkPipelineLayout m_Layout = VK_NULL_HANDLE;
			
			VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
			
			VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;




			PerFrameResource<VkDescriptorSet> m_UniformBufferDescriptorSets;


			bool m_Finalized = false;
			

			// Inherited via GraphicsPipeline
			void SetUniformBuffer(Kaidel::Ref<Kaidel::UniformBuffer> uniformBuffer, uint32_t binding = 0U) override;


			// Inherited via GraphicsPipeline
			void Bind() override;

			void Unbind() override;

		};

	}
}

