#pragma once
#include "Kaidel/Renderer/GraphicsAPI/SamplerState.h"

#include <glad/vulkan.h>

namespace Kaidel {
	class VulkanSamplerState : public SamplerState {
	public:
		VulkanSamplerState(const SamplerParameters& params);
		~VulkanSamplerState();

		const SamplerParameters& GetParameters() const override { return m_Parameters; }
		RendererID GetRendererID() const override { return (RendererID)m_Sampler; }

	private:
		SamplerParameters m_Parameters;
		VkSampler m_Sampler;
	};
}
