#include "KDpch.h"
#include "TextureView.h"
#include "Kaidel/Renderer/RendererAPI.h"
#include "Kaidel/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLTextureView.h"

namespace Kaidel {
	Ref<TextureView> TextureView::Create(const TextureHandle& handle){
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTextureView>(handle);
			//case RendererAPI::API::DirectX: return CreateRef<D3DTexture2D>(width, height);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<TextureView> TextureView::Create(const TextureArrayHandle& handle) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTextureView>(handle);
			//case RendererAPI::API::DirectX: return CreateRef<D3DTexture2D>(width, height);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
