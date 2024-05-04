#pragma once

#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"



namespace Kaidel {
	namespace Vulkan {

		class VulkanGraphicsPipeline : public GraphicsPipeline {
		public:
			VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specification);
			~VulkanGraphicsPipeline();

			void Finalize() override;
		private:
			GraphicsPipelineSpecification m_Specification;
			VkPipeline m_Pipeline = VK_NULL_HANDLE;
			VkPipelineLayout m_Layout = VK_NULL_HANDLE;
			bool m_Finalized = false;
			
		};

	}
}

