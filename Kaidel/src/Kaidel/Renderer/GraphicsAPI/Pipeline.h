#pragma once

#include "Kaidel/Core/Base.h"
#include "SingleShader.h"
#include "VertexArray.h"
#include "Kaidel/Renderer/Settings.h"

namespace Kaidel {
	enum class PipelineBindingPoint {
		None = 0,
		Graphics,
		Compute,
	};

	enum class Topology {
		None = 0,
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		TriangleFan,
		PatchList
	};


	struct PipelineSpecification {
		PipelineBindingPoint BindingPoint;
		std::unordered_map<ShaderType, Ref<SingleShader>> ShaderStages;
		Topology PrimitiveTopology;
		uint32_t ViewportWidth, ViewportHeight;
		glm::vec4 BlendValues;
		CullMode CullingMode = CullMode::None;
		bool FrontFaceCCW = true;
	};


	class Pipeline : public IRCCounter<false>{
	public:
		static Ref<Pipeline> Create(const PipelineSpecification& specification);

	private:

	};
}

