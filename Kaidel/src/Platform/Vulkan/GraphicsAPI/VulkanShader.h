#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

#include "Platform/Vulkan/VulkanDefinitions.h"


namespace Kaidel {
	class VulkanShader : public ShaderModule {
	public:
		VulkanShader(const ShaderSpecification& spec);
		~VulkanShader();

		void Bind() const override;

		void Unbind() const override;

		RendererID GetRendererID() const override { return (RendererID)m_Module; }

		const ShaderSpecification& GetSpecification() const override { return m_Specification; }


		ShaderReflection& Reflect()override;


	private:
		VkShaderModule m_Module;
		ShaderSpecification m_Specification;

		ShaderReflection m_Reflection;

	};
}
