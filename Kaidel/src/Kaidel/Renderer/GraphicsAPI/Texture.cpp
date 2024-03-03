#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/D3D/D3DTexture.h"
namespace Kaidel {

	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, TextureFormat format)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTexture2D>(width, height,format);
			//case RendererAPI::API::DirectX: return CreateRef<D3DTexture2D>(width, height);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{

		if (s_Map.find(path) == s_Map.end()) {
			switch (Renderer::GetAPI())
			{
				case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
				case RendererAPI::API::OpenGL:  return s_Map[path]=CreateRef<OpenGLTexture2D>(path);
				//case RendererAPI::API::DirectX: return CreateRef<D3DTexture2D>(path);
			}
		}
		else {
			return s_Map.at(path);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}








	std::unordered_map < std::string, Ref<Texture2D>> Texture2D::s_Map;




}
