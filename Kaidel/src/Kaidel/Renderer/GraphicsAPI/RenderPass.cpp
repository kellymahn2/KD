#include "KDpch.h"
#include "RenderPass.h"

#include "temp.h"
#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanRenderPass.h"

namespace Kaidel {
	TMAKE(Ref<RenderPass>, RenderPass::Create, TPACK(const RenderPassSpecification& specs), CreateRef<VulkanRenderPass>, TPACK(specs))
}
