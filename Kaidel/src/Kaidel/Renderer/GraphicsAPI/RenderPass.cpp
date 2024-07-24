#include "KDpch.h"
#include "RenderPass.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanRenderPass.h"

#include "Kaidel/Renderer/RendererAPI.h"

namespace Kaidel {
	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& specification)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:return CreateRef<VulkanRenderPass>(specification);
		}

		KD_CORE_ASSERT(false, "Unknown renderer api");
		return {};
	}
}
