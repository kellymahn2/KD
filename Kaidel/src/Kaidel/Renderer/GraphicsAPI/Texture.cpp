#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanTexture.h"

namespace Kaidel {
	Ref<Texture2D> Texture2D::Create(const Texture2DSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:return CreateRef<VulkanTexture2D>(spec);
		}

		KD_CORE_ASSERT(false, "Unknown renderer api");
		return {};
	}

	Ref<TextureLayered2D> TextureLayered2D::Create(const TextureLayered2DSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:return CreateRef<VulkanTextureLayered2D>(spec);
		}

		KD_CORE_ASSERT(false, "Unknown renderer api");
		return {};
	}

}
