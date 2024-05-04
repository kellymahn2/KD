#include "KDpch.h"
#include "SingleShader.h"
#include "Kaidel/Renderer/RendererAPI.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanSingleShader.h"

namespace Kaidel {
	Ref<SingleShader> SingleShader::CreateShader(const SingleShaderSpecification& specification)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return {};
		case RendererAPI::API::Vulkan:return CreateRef<Vulkan::VulkanSingleShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown renderer api");
		return {};
	}

}
