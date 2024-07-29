#pragma once

#include "Kaidel/Renderer/RendererDefinitions.h"

#include "Shader.h"
#include "RenderPass.h"


namespace Kaidel
{

	struct VertexInputElement {
		std::string Name;
		VertexInputType InputType;
		uint32_t Size;

		VertexInputElement(const std::string& name, VertexInputType inputType)
			:Name(name), InputType(inputType),Size(0)
		{}
	};


	struct VertexInpuBinding {
		std::vector<VertexInputElement> Elements;
		VertexInputRate InputRate;
	};

	struct VertexInputSpecification {
		std::vector<VertexInpuBinding> Bindings;
	};

	struct UniformBufferInputSpecification {
		std::vector<uint32_t> UniformBufferBindings;
	};


	struct GraphicsPipelineSpecification {
		std::string Name;
		VertexInputSpecification InputSpecification;
		Ref<ShaderModule> VertexShader;
		Ref<ShaderModule> TesselationControlShader;
		Ref<ShaderModule> TesselationEvaluationShader;
		Ref<ShaderModule> GeometryShader;
		Ref<ShaderModule> FragmentShader;
		PipelineCullMode CullMode;
		bool FrontCCW = true;
		Ref<RenderPass> UsedRenderPass;

		UniformBufferInputSpecification UniformBufferInput;
	};

	class GraphicsPipeline : public IRCCounter<false> 
	{
	public:
		virtual ~GraphicsPipeline() = default;
		virtual GraphicsPipelineSpecification& GetSpecification() = 0;

		virtual void Recreate() = 0;

		virtual RendererID GetRendererID()const = 0;

		static Ref<GraphicsPipeline> Create(const GraphicsPipelineSpecification& spec);
	};
}
