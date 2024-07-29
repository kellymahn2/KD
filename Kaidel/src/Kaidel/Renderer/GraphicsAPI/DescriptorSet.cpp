#include "KDpch.h"
#include "DescriptorSet.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanDescriptorSet.h"

namespace Kaidel {
	Ref<DescriptorSet> DescriptorSet::Create(Ref<GraphicsPipeline> pipeline, uint32_t setBinding)
	{
		switch (Renderer::GetAPI())
		{
			//case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(specification);
		case RendererAPI::API::Vulkan: return CreateRef<VulkanDescriptorSet>(pipeline,setBinding);
			//case RendererAPI::API::DirectX: return CreateRef<D3DShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<DescriptorSet> DescriptorSet::Create(DescriptorType type, ShaderStages flags)
	{
		switch (Renderer::GetAPI())
		{
			//case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(specification);
		case RendererAPI::API::Vulkan: return CreateRef<VulkanDescriptorSet>(type, flags);
			//case RendererAPI::API::DirectX: return CreateRef<D3DShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	
}
