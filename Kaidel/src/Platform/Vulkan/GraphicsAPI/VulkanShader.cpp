#include "KDpch.h"
#include "VulkanShader.h"

#include "VulkanGraphicsContext.h"

#include <spirv_cross/spirv_reflect.hpp>

namespace Kaidel {

	namespace Utils {
		static void ReflectDescriptor(ShaderReflection& reflection, const spirv_cross::CompilerReflection& compilerReflection, 
										const spirv_cross::SmallVector<spirv_cross::Resource>& resources, DescriptorType type, uint32_t stageFlags) 
		{
			for (auto& resource : resources) {
				uint32_t set = compilerReflection.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t binding = compilerReflection.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t count = 1;
				const spirv_cross::SPIRType& spirvType = compilerReflection.get_type(resource.type_id);
				if (spirvType.array.size() > 0) {
					count = spirvType.array[0];
				}
				reflection.AddDescriptor(type, count, set, binding);
			}
		}

		static void ReflectPushConstant(ShaderReflection& reflection, const spirv_cross::CompilerReflection& compilerReflection,
											const spirv_cross::SmallVector<spirv_cross::Resource>& resources, ShaderType shaderType) 
		{
			for (auto& resource : resources) {
				const auto& type = compilerReflection.get_type(resource.base_type_id);
				uint32_t size = compilerReflection.get_declared_struct_size(type);
				reflection.AddPushConstant(size, shaderType);
			}
		}

	}

	VulkanShader::VulkanShader(const ShaderSpecification& spec)
		:m_Specification(spec)
	{
		VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		moduleInfo.pCode = spec.SPIRV.data();
		moduleInfo.codeSize = spec.SPIRV.size() * sizeof(uint32_t);

		VK_ASSERT(vkCreateShaderModule(VK_DEVICE.GetDevice(), &moduleInfo, nullptr, &m_Module));

		m_Specification.SPIRV.clear();

		{
			spirv_cross::CompilerReflection reflection(spec.SPIRV);
			
			auto shaderResources = reflection.get_shader_resources();

			Utils::ReflectDescriptor(m_Reflection, reflection, shaderResources.uniform_buffers, DescriptorType::UniformBuffer, 0);
			Utils::ReflectDescriptor(m_Reflection, reflection, shaderResources.storage_buffers, DescriptorType::StorageBuffer, 0);
			Utils::ReflectDescriptor(m_Reflection, reflection, shaderResources.sampled_images, DescriptorType::CombinedSampler, 0);
			Utils::ReflectDescriptor(m_Reflection, reflection, shaderResources.separate_images, DescriptorType::Texture, 0);
			Utils::ReflectDescriptor(m_Reflection, reflection, shaderResources.separate_samplers, DescriptorType::Sampler, 0);
			Utils::ReflectPushConstant(m_Reflection, reflection, shaderResources.push_constant_buffers, spec.Type);
		}
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
	ShaderReflection& VulkanShader::Reflect()
	{
		return m_Reflection;
	}
}
