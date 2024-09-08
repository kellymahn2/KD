#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Sampler.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanSampler : public Sampler {
	public:
		VulkanSampler(const SamplerState& state);
		~VulkanSampler();

		virtual const SamplerState& GetSamplerState() const override { return m_State; }
		VkSampler GetSampler()const { return m_Sampler; }
	private:
		SamplerState m_State;
		VkSampler m_Sampler;
	};
}
