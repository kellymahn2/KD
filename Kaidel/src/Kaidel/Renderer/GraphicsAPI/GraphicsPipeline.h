#pragma once
#include "Kaidel/Core/Base.h"
#include "Shader.h"
#include "RenderPass.h"

namespace Kaidel {

	struct VertexInputElement
	{
		std::string Name;
		Format AttribFormat;
		bool IsDummy = false;
		uint32_t Size;

		VertexInputElement(const std::string& name, Format format)
			:Name(name), AttribFormat(format), Size(0)
		{}

		VertexInputElement(uint32_t size)
			:IsDummy(true), Size(size)
		{}
	};

	struct VertexInputBinding
	{
		std::vector<VertexInputElement> Elements;
		VertexInputRate InputRate;

		VertexInputBinding() = default;
		VertexInputBinding(std::initializer_list<VertexInputElement> elements,VertexInputRate inputRate = VertexInputRate::Vertex)
			:Elements(elements),InputRate(inputRate)
		{}
	};

	struct VertexInputSpecification
	{
		std::vector<VertexInputBinding> Bindings;
	};

	struct PipelineRasterization
	{
		bool DepthClamp = false;
		bool DiscardPrimitives = false;
		bool Wireframe = false;
		PipelineCullMode CullMode = PipelineCullMode::None;
		bool FrontCCW = true;
		bool DepthBias = false;
		float DepthBiasConstant = 0.0f;
		float DepthBiasClamp = 0.0f;
		float DepthBiasSlope = 0.0f;
		float LineWidth = 1.0f;
		uint32_t PatchControlPoints = 1;
	};

	struct PipelineMultisample
	{
		TextureSamples Samples = TextureSamples::x1;
		bool SampleShading = false;
		float SampleShadingMin = 0.0f;
		std::vector<uint32_t> SampleMasks;
		bool AlphaToCoverage = false;
		bool AlphaToOne = false;
	};

	struct PipelineDepthStencil
	{
		bool DepthTest = false;
		bool DepthWrite = false;
		CompareOp DepthCompareOperator = CompareOp::Less;
		bool DepthRange = false;
		float DepthRangeMin = 0;
		float DepthRangeMax = 0;
		bool Stencil = false;

		struct StencilOperation {
			StencilOp Fail = StencilOp::Zero;
			StencilOp Pass = StencilOp::Zero;
			StencilOp DepthFail = StencilOp::Zero;
			CompareOp Compare = CompareOp::Always;
			uint32_t CompareMask = 0;
			uint32_t WriteMask = 0;
			uint32_t Reference = 0;
		};

		StencilOperation StencilFrontOp;
		StencilOperation StencilBackOp;
	};

	struct PipelineColorBlend
	{
		bool LogicOpEnable = false;
		LogicOp LogicOp = LogicOp::Clear;

		struct Attachment
		{
			uint32_t AttachmentIndex = -1;
			bool Blend = false;
			BlendFactor SrcColorBlend = BlendFactor::Zero;
			BlendFactor DstColorBlend = BlendFactor::Zero;
			BlendOp ColorBlendOp = BlendOp::Add;
			BlendFactor SrcAlphaBlend = BlendFactor::Zero;
			BlendFactor DstAlphaBlend = BlendFactor::Zero;
			BlendOp AlphaBlendOp = BlendOp::Add;
			bool WriteR = true;
			bool WriteG = true;
			bool WriteB = true;
			bool WriteA = true;
		};

		std::vector<Attachment> Attachments;
		float BlendConstant[4] = { 0.0f };
	};

	enum PipelineDynamicState_
	{
		PipelineDynamicState_LineWidth = (1 << 0),
		PipelineDynamicState_DepthBias = (1 << 1),
		PipelineDynamicState_BlendConstants = (1 << 2),
		PipelineDynamicState_DepthBounds = (1 << 3),
		PipelineDynamicState_StencilCompareMask = (1 << 4),
		PipelineDynamicState_StencilWriteMask = (1 << 5),
		PipelineDynamicState_StencilReference = (1 << 6),
	};

	typedef int PipelineDynamicState;

	struct GraphicsPipelineSpecification {
		Ref<Shader> Shader;
		VertexInputSpecification Input;
		PrimitiveTopology Primitive;
		PipelineRasterization Rasterization;
		PipelineMultisample Multisample;
		PipelineDepthStencil DepthStencil;
		PipelineColorBlend Blend;
		PipelineDynamicState DynamicStates = 0;
		Ref<RenderPass> RenderPass;
		uint32_t Subpass = 0;
	};


	class GraphicsPipeline : public IRCCounter<false> {
	public:

		virtual ~GraphicsPipeline() = default;
		static Ref<GraphicsPipeline> Create(const GraphicsPipelineSpecification& specification);
	};

}
