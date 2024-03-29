#include "KDpch.h"
#include "CubeMap.h"
#include "Kaidel/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLCubeMap.h"
namespace Kaidel {
	Ref<CubeMap> CubeMap::Create(uint32_t width, uint32_t height, TextureFormat format) {
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL: return CreateRef<OpenGLCubeMap>(width, height, format);
		}

		KD_CORE_ASSERT(false, "Unknown Renderer API!");
		return nullptr;
	}
}
