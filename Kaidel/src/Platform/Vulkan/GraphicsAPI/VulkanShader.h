#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanShader : public Shader {
	public:
		VulkanShader(const std::unordered_map<ShaderType, Spirv>& spirvs);
		~VulkanShader();

		auto& GetShaderInfo()const { return m_Shader; }

	private:
		VulkanBackend::ShaderInfo m_Shader;
		friend class VulkanRendererAPI;
	};
}
