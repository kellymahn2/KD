#include "KDpch.h"
#include "VulkanComputePipeline.h"
#include "VulkanGraphicsContext.h"
#include "VulkanShader.h"

namespace Kaidel {
	VulkanComputePipeline::VulkanComputePipeline(Ref<Shader> shader)
		:m_Shader(shader)
	{
		Ref<VulkanShader> vs = shader;
		m_Pipeline = VK_BACKEND->CreateComputePipeline(vs->GetShaderInfo());
	}
	VulkanComputePipeline::~VulkanComputePipeline()
	{
		VK_BACKEND->DestroyComputePipeline(m_Pipeline);
	}
}
