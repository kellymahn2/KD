#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/D3D/D3DVertexArray.h"
namespace Kaidel {

	Ref<VertexArray> VertexArray::Create(const VertexArraySpecification& spec) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexArray>(spec);
		//case RendererAPI::API::DirectX: return CreateRef<D3DVertexArray>(spec);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI");
		return nullptr;
	}
}
