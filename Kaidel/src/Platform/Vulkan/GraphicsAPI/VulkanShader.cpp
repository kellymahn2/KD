#include "KDpch.h"
#include "VulkanShader.h"
#include "VulkanGraphicsContext.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "VulkanGraphicsPipeline.h"

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

	void VulkanShader::Update(const std::unordered_map<ShaderType, Spirv>& spirvs)
	{
		std::unordered_map<VkShaderStageFlagBits, std::initializer_list<uint32_t>> vkSpirvs;

		for (auto& [shader, spirv] : spirvs) {
			auto beg = spirv.data();
			auto end = spirv.data() + spirv.size();

			vkSpirvs[Utils::ShaderTypeToVulkanShaderStageFlag(shader)] = std::initializer_list<uint32_t>(beg, end);
		}
		
		VK_BACKEND->UpdateShaderModules(vkSpirvs, m_Shader);

		if (m_Pipelines.empty())
			return;
		RenderCommand::DeviceWaitIdle();
		
		for (uint32_t i = 0; i < m_Pipelines.size(); ++i)
		{
			((VulkanGraphicsPipeline*)m_Pipelines[i].Get())->Recreate();
		}
	}

	void VulkanShader::BindPipeline(Ref<GraphicsPipeline> pipeline)
	{
		m_Pipelines.emplace_back(pipeline);
	}

}
