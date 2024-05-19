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


	enum class GraphicsPipelineInputDataType
	{
		None = 0, Dummy, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	enum class GraphicsPipelineInputRate {
		None = 0,
		PerVertex,
		PerInstance,
	};


	struct GraphicsPipelineInputElement {
		std::string Name;
		GraphicsPipelineInputDataType DataType;
		uint32_t Size;
		uint32_t Offset;
	};


	struct GraphicsPipelineInputBufferSpecification {
		std::vector<GraphicsPipelineInputElement> Elements;
		uint32_t Stride;
		GraphicsPipelineInputRate InputRate;

		void CalculateOffsetsAndStrides() {
			size_t offset = 0;
			Stride = 0;
			for (auto& element : Elements)
			{
				element.Offset = (uint32_t)offset;
				offset += element.Size;
				Stride += element.Size;
			}
		}
	};

	class GraphicsPipelineInputLayout {
	public:
		GraphicsPipelineInputLayout() = default;
		GraphicsPipelineInputLayout(const std::vector<GraphicsPipelineInputBufferSpecification>& bufferSpecifications)
			:m_BufferSpecifications(bufferSpecifications)
		{
			CalculateOffsetsAndStrides();
		}

		auto begin() { return m_BufferSpecifications.begin(); }
		auto end() { return m_BufferSpecifications.end(); }
		auto begin() const { return m_BufferSpecifications.begin(); }
		auto end() const { return m_BufferSpecifications.end(); }

		const auto& GetBufferSpecifications() const {
			return m_BufferSpecifications;
		}



	private:
		void CalculateOffsetsAndStrides() {
			for (auto& bufferSpecification : m_BufferSpecifications) {
				bufferSpecification.CalculateOffsetsAndStrides();
			}
		}
		std::vector<GraphicsPipelineInputBufferSpecification> m_BufferSpecifications;
	};


	struct GraphicsPipelineSpecification {
		GraphicsPipelineInputLayout InputLayout;
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
