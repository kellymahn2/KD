#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Backend.h"

namespace Kaidel {

	class GraphicsPipeline;

	class VulkanShader : public Shader {
	public:
		VulkanShader(const std::unordered_map<ShaderType, Spirv>& spirvs);
		~VulkanShader();

		auto& GetShaderInfo()const { return m_Shader; }
		virtual uint32_t GetSetCount()const override { return (uint32_t)m_Shader.DescriptorSetLayouts.size(); }
		
		virtual void Update(const std::unordered_map<ShaderType, Spirv>& spirvs) override;

		virtual void BindPipeline(Ref<GraphicsPipeline> pipeline) override;

		virtual const ShaderReflection& GetReflection() const override { return m_Shader.Reflection; }

	private:
		VulkanBackend::ShaderInfo m_Shader;
		std::vector<Ref<GraphicsPipeline>> m_Pipelines;
		friend class VulkanRendererAPI;
	};
}
