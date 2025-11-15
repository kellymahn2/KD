#pragma once
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanGraphicsPipeline : public GraphicsPipeline {
	public:
		VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specs);
		~VulkanGraphicsPipeline();
		VkPipeline GetPipeline()const { return m_Pipeline; }
		void Recreate();
	private:
		GraphicsPipelineSpecification m_Specification;
		VkPipeline m_Pipeline;
	};
}
