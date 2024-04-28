#include "KDpch.h"
#include "RenderPass.h"

#include "Kaidel/Renderer/RendererAPI.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanRenderPass.h"

namespace Kaidel {
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& specification)
	{

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:return CreateRef<Vulkan::VulkanRenderPass>(specification);
		}

		return {};
	}
}
