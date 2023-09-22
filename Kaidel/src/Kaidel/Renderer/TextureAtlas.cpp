#include "KDpch.h"

#include "TextureAtlas.h"
#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLTextureAtlas.h"
namespace Kaidel {


	/*Ref<Texture2D> TextureAtlas2D::Create(uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTextureAtlas2D>(width, height);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;


	}*/


	Ref<TextureAtlas2D> TextureAtlas2D::Create(const std::string& path)
	{
		if (s_Map.find(path) == s_Map.end()) {

			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return s_Map[path] = CreateRef<OpenGLTextureAtlas2D>(path);
			}
		}
		else {
			return s_Map.at(path);
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}


	std::unordered_map < std::string, Ref<TextureAtlas2D>> TextureAtlas2D::s_Map;

}
