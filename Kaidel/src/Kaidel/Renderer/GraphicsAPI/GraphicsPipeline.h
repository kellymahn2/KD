#pragma once


#include "Kaidel/Core/Base.h"
#include "Buffer.h"
#include "SingleShader.h"
#include "Kaidel/Renderer/Settings.h"
#include "RenderPass.h"


namespace Kaidel {


	enum class GraphicsPrimitveTopology {
		None = 0,
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		PatchList,
	};

	struct GraphicsPipelineViewport {
		float Width = 0.0f,Height = 0.0f;
		float TopLeftX = 0.0f, TopLeftY = 0.0f;
		float MinDepth = 0.0f,MaxDepth = 1.0f;
	};

	struct GraphicsPipelineSpecification {
		BufferLayout InputLayout;
		GraphicsPrimitveTopology PrimitveTopology = GraphicsPrimitveTopology::TriangleList;
		std::unordered_map<ShaderType, Ref<SingleShader>> Stages;
		GraphicsPipelineViewport Viewport{};
		CullMode Culling = CullMode::None;
		bool FrontCCW = true;
		float LineWidth = 1.0f;
		Ref<RenderPass> RenderPass;
	};


	class GraphicsPipeline : public IRCCounter<false> {
	public:

		virtual ~GraphicsPipeline() = default;
		virtual void Finalize() = 0;


		static Ref<GraphicsPipeline> Create(const GraphicsPipelineSpecification& specification);
	private:

	};

}
