#include "KDpch.h"
#include "SamplerState.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanSamplerState.h"

namespace Kaidel {
	Ref<SamplerState> SamplerState::Create(const SamplerParameters& params)
	{
		switch (Renderer::GetAPI())
		{
			//case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(specification);
			case RendererAPI::API::Vulkan: return CreateRef<VulkanSamplerState>(params);
			//case RendererAPI::API::DirectX: return CreateRef<D3DShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
