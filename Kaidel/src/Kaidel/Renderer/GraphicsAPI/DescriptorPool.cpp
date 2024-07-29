#include "KDpch.h"
#include "DescriptorPool.h"
#include "Kaidel/Renderer/Renderer.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanDescriptorPool.h"

namespace Kaidel {
	Ref<DescriptorPool> DescriptorPool::Create(const std::vector<DescriptorPoolSize>& sizes, uint32_t maxSetCount, uint32_t flags)
	{
		switch (Renderer::GetAPI())
		{
			//case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(specification);
		case RendererAPI::API::Vulkan: return CreateRef<VulkanDescriptorPool>(sizes,maxSetCount,flags);
			//case RendererAPI::API::DirectX: return CreateRef<D3DShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
