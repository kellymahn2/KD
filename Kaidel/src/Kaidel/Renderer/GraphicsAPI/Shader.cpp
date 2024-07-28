#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanShader.h"


namespace Kaidel {


	Ref<Shader> Shader::Create(const ShaderSpecification& specification) {
		switch (Renderer::GetAPI())
		{
		//case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(specification);
		case RendererAPI::API::Vulkan: return CreateRef<VulkanShader>(specification);
		//case RendererAPI::API::DirectX: return CreateRef<D3DShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
