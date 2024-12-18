#include "KDpch.h"
#include "Shader.h"

#include "temp.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanShader.h"

namespace Kaidel {
	TMAKE(Ref<Shader>, Shader::Create, TPACK(const std::unordered_map<ShaderType, Spirv>& spirvs), CreateRef<VulkanShader>, TPACK(spirvs));
}
