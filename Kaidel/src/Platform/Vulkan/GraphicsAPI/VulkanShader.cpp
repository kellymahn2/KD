#include "KDpch.h"
#include "VulkanShader.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	VulkanShader::VulkanShader(const std::unordered_map<ShaderType, Spirv>& spirvs)
	{

		std::unordered_map<VkShaderStageFlagBits, std::initializer_list<uint32_t>> vkSpirvs;
		
		for (auto& [shader, spirv] : spirvs) {
			auto beg = spirv.data();
			auto end = spirv.data() + spirv.size();
			
			vkSpirvs[Utils::ShaderTypeToVulkanShaderStageFlag(shader)] = std::initializer_list<uint32_t>(beg,end);
		}

		m_Shader = VK_CONTEXT.GetBackend()->CreateShader(vkSpirvs);
	}
	VulkanShader::~VulkanShader()
	{
		VK_CONTEXT.GetBackend()->DestroyShader(m_Shader);
	}
}
