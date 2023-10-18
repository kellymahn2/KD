#include "KDpch.h"
#include "Kaidel/Renderer/VertexArray.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/D3D/D3DVertexArray.h"
namespace Kaidel {

	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexArray>();
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<VertexArray> VertexArray::Create(Ref<Shader> shader) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::DirectX: return CreateRef<D3DVertexArray>(shader);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
