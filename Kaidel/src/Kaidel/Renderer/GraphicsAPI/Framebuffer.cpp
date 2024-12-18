#include "KDpch.h"
#include "Framebuffer.h"

#include "temp.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanFramebuffer.h"

namespace Kaidel {
	TMAKE(Ref<Framebuffer>, Framebuffer::Create, TPACK(const FramebufferSpecification& spec), CreateRef<VulkanFramebuffer>, TPACK(spec))
}
