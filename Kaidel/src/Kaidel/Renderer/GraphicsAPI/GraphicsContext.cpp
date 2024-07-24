#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanGraphicsContext.h"
namespace Kaidel {

	Scope<GraphicsContext> GraphicsContext::Create(Window* window)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			//case RendererAPI::API::OpenGL:  return CreateScope<OpenGLContext>(window);
			case RendererAPI::API::Vulkan: return CreateScope<VulkanGraphicsContext>(window);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
