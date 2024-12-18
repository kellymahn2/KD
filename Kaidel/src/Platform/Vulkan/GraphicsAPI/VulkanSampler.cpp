#include "KDpch.h"
#include "VulkanSampler.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanSampler::VulkanSampler(const SamplerState& state)
		:m_State(state)
	{
		VulkanBackend::SamplerState backendState{};
		backendState.AddressModeU = Utils::SamplerAddressModeToVulkanAddressMode(state.AddressModeU);
		backendState.AddressModeV = Utils::SamplerAddressModeToVulkanAddressMode(state.AddressModeV);
		backendState.AddressModeW = Utils::SamplerAddressModeToVulkanAddressMode(state.AddressModeW);
		backendState.Aniso = state.Aniso;
		backendState.AnisoMax = state.AnisoMax;
		backendState.BorderColor = Utils::SamplerBorderColorToVulkanBorderColor(state.BorderColor);
		backendState.Compare = state.Compare;
		backendState.CompareOp = Utils::CompareOpToVulkanCompareOp(state.CompareOp);
		backendState.LodBias = state.LodBias;
		backendState.MagFilter = Utils::SamplerFilterToVulkanFilter(state.MagFilter);
		backendState.MinFilter = Utils::SamplerFilterToVulkanFilter(state.MinFilter);
		backendState.MaxLod = state.MaxLod;
		backendState.MinLod = state.MinLod;
		backendState.MipFilter = Utils::SamplerMipMapModeToVulkanMipMapMode(state.MipFilter);
		backendState.Unnormalized = state.Unnormalized;

		m_Sampler = VK_CONTEXT.GetBackend()->CreateSampler(backendState);
	}
	VulkanSampler::~VulkanSampler()
	{
		VK_CONTEXT.GetBackend()->DestroySampler(m_Sampler);
	}
}
