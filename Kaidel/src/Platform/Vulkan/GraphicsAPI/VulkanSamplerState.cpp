#include "KDpch.h"
#include "VulkanSamplerState.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	namespace Utils {
		static VkSampler CreateSampler(VkDevice device, const SamplerParameters& params) {
			VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
			samplerInfo.magFilter = Utils::SamplerFilterToVulkanFilter(params.MagnificationFilter);
			samplerInfo.minFilter = Utils::SamplerFilterToVulkanFilter(params.MinificationFilter);
			samplerInfo.mipmapMode = Utils::SamplerMipMapModeToVulkanMipMapMode(params.MipmapMode);
			samplerInfo.addressModeU = Utils::SamplerAddressModeToVulkanAddressMode(params.AddressModeU);
			samplerInfo.addressModeV = Utils::SamplerAddressModeToVulkanAddressMode(params.AddressModeV);
			samplerInfo.addressModeW = Utils::SamplerAddressModeToVulkanAddressMode(params.AddressModeW);
			samplerInfo.mipLodBias = params.LODBias;
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 0.0f;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.minLod = params.MinLOD;
			samplerInfo.maxLod = params.MaxLOD;
			samplerInfo.borderColor = Utils::SamplerBorderColorToVulkanBorderColor(params.BorderColor);
			samplerInfo.unnormalizedCoordinates = VK_FALSE;

			VkSampler sampler{};
			VK_ASSERT(vkCreateSampler(device, &samplerInfo, nullptr, &sampler));
			return sampler;
		}
	}



	VulkanSamplerState::VulkanSamplerState(const SamplerParameters& params)
		:m_Parameters(params)
	{
		m_Sampler = Utils::CreateSampler(VK_DEVICE.GetDevice(), m_Parameters);
	}
	VulkanSamplerState::~VulkanSamplerState()
	{
		vkDestroySampler(VK_DEVICE.GetDevice(), m_Sampler, nullptr);
	}
}
