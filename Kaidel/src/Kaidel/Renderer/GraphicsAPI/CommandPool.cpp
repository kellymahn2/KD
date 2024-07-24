#include "KDpch.h"
#include "CommandPool.h"


#include "Platform/Vulkan/GraphicsAPI/VulkanCommandPool.h"

#include "Kaidel/Renderer/RendererAPI.h"

namespace Kaidel {
	Ref<CommandPool> CommandPool::Create(CommandPoolOperationType opType, CommandPoolFlags flags)
	{

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:return CreateRef<VulkanCommandPool>(opType, flags);
		}

		KD_CORE_ASSERT("Unknown renderer api");

		return {};
	}
}
