#include "KDpch.h"
#include "TextureArray.h"
#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTextureArray.h"
namespace Kaidel {
	Ref<Texture2DArray> Texture2DArray::Create(uint32_t width, uint32_t height, TextureFormat textureFormat,bool loadsSpecs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTexture2DArray>(width, height,textureFormat,loadsSpecs);
			//case RendererAPI::API::DirectX: return CreateRef<D3DTexture2D>(path);
		case RendererAPI::API::Vulkan: return {};
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
