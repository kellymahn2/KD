#include "KDpch.h"
#include "SingleShader.h"
#include "Kaidel/Renderer/RendererAPI.h"

namespace Kaidel {
	Ref<SingleShader> SingleShader::CreateShader(const SingleShaderSpecification& specification)
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return {};
		}
		KD_CORE_ASSERT(false, "Unknown renderer api");
		return {};
	}

}
