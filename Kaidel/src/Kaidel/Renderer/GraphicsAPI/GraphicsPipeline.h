#pragma once


#include "Kaidel/Core/Base.h"
#include "Buffer.h"
#include "SingleShader.h"
#include "Kaidel/Renderer/Settings.h"
#include "RenderPass.h"
#include "UniformBuffer.h"


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

	static uint32_t PipelineInputDataTypeSize(GraphicsPipelineInputDataType dataType) {
		switch (dataType)
		{
		case GraphicsPipelineInputDataType::Float:return sizeof(float);
		case GraphicsPipelineInputDataType::Float2:return sizeof(float) * 2;
		case GraphicsPipelineInputDataType::Float3:return sizeof(float) * 3;
		case GraphicsPipelineInputDataType::Float4:return sizeof(float) * 4;
		case GraphicsPipelineInputDataType::Mat3:return sizeof(float) * 3;
		case GraphicsPipelineInputDataType::Mat4:return sizeof(float) * 4;
		case GraphicsPipelineInputDataType::Int:return sizeof(int32_t);
		case GraphicsPipelineInputDataType::Int2:return sizeof(int32_t) * 2;
		case GraphicsPipelineInputDataType::Int3:return sizeof(int32_t) * 3;
		case GraphicsPipelineInputDataType::Int4:return sizeof(int32_t) * 4;
		case GraphicsPipelineInputDataType::Bool:return sizeof(int8_t);
		}
		KD_CORE_ASSERT(false, "Unknown input data type");
		return 0;
	}

	struct GraphicsPipelineInputElement {
		std::string Name;
		GraphicsPipelineInputDataType DataType;
		uint32_t Size;
		uint32_t Offset;

		GraphicsPipelineInputElement(const std::string& name, GraphicsPipelineInputDataType dataType)
			:Name(name),DataType(dataType),Size(PipelineInputDataTypeSize(dataType)),Offset(0)
		{}

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

		std::vector<Ref<UniformBuffer>> UsedUniformBuffers;


		uint32_t UniformBufferCount = 0;
		uint32_t StorageBufferCount = 0;
		uint32_t SamplerCount = 0;


	};


	class GraphicsPipeline : public IRCCounter<false> {
	public:

		virtual ~GraphicsPipeline() = default;
		virtual void Finalize() = 0;

		virtual void SetUniformBuffer(Ref<UniformBuffer> uniformBuffer,uint32_t binding = 0) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		//virtual void AddStorageBuffer() = 0;
		//virtual void AddSampler() = 0;



		static Ref<GraphicsPipeline> Create(const GraphicsPipelineSpecification& specification);
	private:

	};

}
