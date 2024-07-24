#include "KDpch.h"
#include "CommandBuffer.h"

#include "Kaidel/Renderer/RendererAPI.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanCommandBuffer.h"


namespace Kaidel {
	Ref<CommandBuffer> CommandBuffer::Create(Ref<CommandPool> commandPool, CommandBufferType type, uint32_t flags)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:return CreateRef<VulkanCommandBuffer>(commandPool, type, flags);
		}

		KD_CORE_ASSERT(false, "Unknown api");
	
		return {};
	}
}
