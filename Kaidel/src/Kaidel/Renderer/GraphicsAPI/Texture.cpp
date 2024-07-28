#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Kaidel {
	Ref<ImmutableTexture2D> ImmutableTexture2D::Create(const Texture2DSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::Vulkan: return {};
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
