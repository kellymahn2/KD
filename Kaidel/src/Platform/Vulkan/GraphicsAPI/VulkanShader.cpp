#include "KDpch.h"
#include "VulkanShader.h"

#include "VulkanGraphicsContext.h"


namespace Kaidel {
	VulkanShader::VulkanShader(const ShaderSpecification& spec)
		:m_Specification(spec)
	{
		VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		moduleInfo.pCode = spec.SPIRV.data();
		moduleInfo.codeSize = spec.SPIRV.size() * sizeof(uint32_t);

		VK_ASSERT(vkCreateShaderModule(VK_DEVICE.GetDevice(), &moduleInfo, nullptr, &m_Module));

		m_Specification.SPIRV.clear();
	}
	VulkanShader::~VulkanShader()
	{
		vkDestroyShaderModule(VK_DEVICE.GetDevice(), m_Module, nullptr);
	}
	void VulkanShader::Bind() const
	{
	}
	void VulkanShader::Unbind() const
	{
	}
}
