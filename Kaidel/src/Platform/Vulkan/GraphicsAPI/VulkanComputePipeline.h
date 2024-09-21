#pragma once
#include "Kaidel/Renderer/GraphicsAPI/ComputePipeline.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanComputePipeline : public ComputePipeline {
	public:
		VulkanComputePipeline(Ref<Shader> shader);
		~VulkanComputePipeline();

		VkPipeline GetPipeline()const { return m_Pipeline; }
	private:
		VkPipeline m_Pipeline;
		Ref<Shader> m_Shader;
	};
}

