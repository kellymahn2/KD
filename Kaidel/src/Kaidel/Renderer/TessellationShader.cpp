#include "KDpch.h"
#include "TessellationShader.h"
#include "RendererAPI.h"

#include "Platform/OpenGL/OpenGLTessellationShader.h"

namespace Kaidel {
	Ref<TessellationShader> TessellationShader::Create(const FileSystem::path& tcsPath, const FileSystem::path& tesPath) {
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:return CreateRef<OpenGLTessellationShader>(tcsPath,tesPath);
		}

		return {};
	}
}
