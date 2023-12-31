#include "KDpch.h"
#include "Kaidel/Renderer/Framebuffer.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/D3D/D3DFramebuffer.h"

namespace Kaidel {
	

	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFramebuffer>(spec);
			//case RendererAPI::API::DirectX: return CreateRef<D3DFrameBuffer>(spec);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

