#include "KDpch.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanGraphicsContext.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "VulkanSingleShader.h"
#include "VulkanRenderPass.h"


namespace Kaidel {

	namespace Utils {
		static VkFormat KaidelShaderDataTypeToVkFormat(ShaderDataType type) {
			switch (type)
			{
			case Kaidel::ShaderDataType::Float:return VK_FORMAT_R32_SFLOAT;
			case Kaidel::ShaderDataType::Float2:return VK_FORMAT_R32G32_SFLOAT;
			case Kaidel::ShaderDataType::Float3:return VK_FORMAT_R32G32B32_SFLOAT;
			case Kaidel::ShaderDataType::Float4:return VK_FORMAT_R32G32B32A32_SFLOAT;
			case Kaidel::ShaderDataType::Int:return VK_FORMAT_R32_SINT;
			case Kaidel::ShaderDataType::Int2:return VK_FORMAT_R32G32_SINT;
			case Kaidel::ShaderDataType::Int3:return VK_FORMAT_R32G32B32_SINT;
			case Kaidel::ShaderDataType::Int4:return VK_FORMAT_R32G32B32A32_SINT;
			case Kaidel::ShaderDataType::Bool:return VK_FORMAT_R8_SINT;
			case Kaidel::ShaderDataType::Mat3:return VK_FORMAT_R32G32B32_SFLOAT;
			case Kaidel::ShaderDataType::Mat4:return VK_FORMAT_R32G32B32A32_SFLOAT;
			}
			KD_CORE_ASSERT(false, "Unsupported shader data type");
			return VK_FORMAT_UNDEFINED;
		}

		static VkVertexInputRate KaidelInputRateToVkInputRate(GraphicsPipelineInputRate inputRate) {
			switch (inputRate)
			{
			case Kaidel::GraphicsPipelineInputRate::PerVertex:return VK_VERTEX_INPUT_RATE_VERTEX;
			case Kaidel::GraphicsPipelineInputRate::PerInstance:return VK_VERTEX_INPUT_RATE_INSTANCE;
			}

			KD_CORE_ASSERT(false, "Unknown input rate");
			return VK_VERTEX_INPUT_RATE_MAX_ENUM;
		}

		struct VertexInputStateCreateResult {
			VkPipelineVertexInputStateCreateInfo CreateInfo;
			std::vector<VkVertexInputAttributeDescription> AttributeDescs;
			std::vector<VkVertexInputBindingDescription> BindingDescs;
		};
		static VkVertexInputAttributeDescription MakeInputDescription(uint32_t binding,uint32_t location,VkFormat format, uint32_t offset) {
			VkVertexInputAttributeDescription attribute{};
			attribute.location = location;
			attribute.binding = 0;
			attribute.format = format;
			attribute.offset = offset;
			return attribute;
		}

		static VertexInputStateCreateResult CreateVertexInputState(const GraphicsPipelineInputLayout& layouts) {

			VertexInputStateCreateResult result{};
			result.CreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			uint32_t bindingIndex = 0;
			uint32_t attributeIndex = 0;
			
			for (auto& bufferSpec : layouts) {

				//Binding
				{
					VkVertexInputBindingDescription binding{};
					binding.binding = bindingIndex;
					binding.inputRate = KaidelInputRateToVkInputRate(bufferSpec.InputRate);
					binding.stride = bufferSpec.Stride;
					result.BindingDescs.push_back(binding);
				}

				//Atrributes
				{
					for (auto& element : bufferSpec.Elements) {

						if (element.DataType == GraphicsPipelineInputDataType::Dummy)
							continue;

						VkFormat format = VK_FORMAT_UNDEFINED;
						uint32_t offset = element.Offset;

						switch (element.DataType)
						{

						case GraphicsPipelineInputDataType::Float: format = VK_FORMAT_R32_SFLOAT; break;
						case GraphicsPipelineInputDataType::Float2: format = VK_FORMAT_R32G32_SFLOAT; break;
						case GraphicsPipelineInputDataType::Float3:format = VK_FORMAT_R32G32B32_SFLOAT; break;
						case GraphicsPipelineInputDataType::Float4:format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
						case GraphicsPipelineInputDataType::Int: format = VK_FORMAT_R32_SINT; break;
						case GraphicsPipelineInputDataType::Int2:  format = VK_FORMAT_R32G32_SINT; break;
						case GraphicsPipelineInputDataType::Int3:  format = VK_FORMAT_R32G32B32_SINT; break;
						case GraphicsPipelineInputDataType::Int4:  format = VK_FORMAT_R32G32B32A32_SINT; break;
						case GraphicsPipelineInputDataType::Bool:  format = VK_FORMAT_R8_SINT; break;
						}

						result.AttributeDescs.push_back(MakeInputDescription(bindingIndex, attributeIndex, format, offset));
						++attributeIndex;
					}
				}

				++bindingIndex;
			}



			result.CreateInfo.pVertexAttributeDescriptions = result.AttributeDescs.data();
			result.CreateInfo.vertexAttributeDescriptionCount = (uint32_t)result.AttributeDescs.size();
			result.CreateInfo.pVertexBindingDescriptions = result.BindingDescs.data();
			result.CreateInfo.vertexBindingDescriptionCount = (uint32_t)result.BindingDescs.size();

			return result;
		}

		static VkPrimitiveTopology KaidelPrimitiveTopologyToVkPrimitiveTopology(GraphicsPrimitveTopology topology) {
			switch (topology)
			{
			case Kaidel::GraphicsPrimitveTopology::PointList:return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case Kaidel::GraphicsPrimitveTopology::LineList:return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case Kaidel::GraphicsPrimitveTopology::LineStrip:return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case Kaidel::GraphicsPrimitveTopology::TriangleList:return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case Kaidel::GraphicsPrimitveTopology::TriangleStrip:return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case Kaidel::GraphicsPrimitveTopology::TriangleFan:return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			case Kaidel::GraphicsPrimitveTopology::PatchList:return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			}
			KD_CORE_ASSERT(false, "Unknown topology type");
			return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
		}

		static VkShaderStageFlagBits KaidelShaderStageToVkShaderStage(ShaderType type) {
			switch (type)
			{
			case Kaidel::ShaderType::VertexShader:return VK_SHADER_STAGE_VERTEX_BIT;
			case Kaidel::ShaderType::FragmentShader: return VK_SHADER_STAGE_FRAGMENT_BIT;
			case Kaidel::ShaderType::GeometryShader: return VK_SHADER_STAGE_GEOMETRY_BIT;
			case Kaidel::ShaderType::TessellationControlShader: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			case Kaidel::ShaderType::TessellationEvaluationShader: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			}
			KD_CORE_ASSERT(false,"Unknown shader type");
			return VK_SHADER_STAGE_ALL;
		}

		static VkCullModeFlagBits KaidelCullModeToVkCullMode(CullMode mode) {
			switch (mode)
			{
			case Kaidel::CullMode::None: return VK_CULL_MODE_NONE;
			case Kaidel::CullMode::Front:return VK_CULL_MODE_FRONT_BIT;
			case Kaidel::CullMode::Back:return VK_CULL_MODE_BACK_BIT;
			case Kaidel::CullMode::FrontAndBack:return VK_CULL_MODE_FRONT_AND_BACK;
			}

			KD_CORE_ASSERT(false, "Unknown culling mode");
			return VK_CULL_MODE_NONE;
		}
	}

	namespace Vulkan {
		
		VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specification)
			:m_Specification(specification)
		{
		}
		VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
		{
			if (m_Finalized) {
				vkDestroyPipelineLayout(VK_DEVICE, m_Layout, VK_ALLOCATOR_PTR);
				vkDestroyPipeline(VK_DEVICE, m_Pipeline, VK_ALLOCATOR_PTR);
			}
		}
		void VulkanGraphicsPipeline::Finalize()
		{
			VK_STRUCT(VkGraphicsPipelineCreateInfo, pipelineInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
			pipelineInfo.basePipelineIndex = 0;

			//Vertex Input
			auto vertexInputCreateResult = Utils::CreateVertexInputState(m_Specification.InputLayout);
			pipelineInfo.pVertexInputState = &vertexInputCreateResult.CreateInfo;

			//Input Assembly
			VK_STRUCT(VkPipelineInputAssemblyStateCreateInfo, inputAssemblyInfo, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
			inputAssemblyInfo.topology = Utils::KaidelPrimitiveTopologyToVkPrimitiveTopology(m_Specification.PrimitveTopology);
			pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

			//Shader Stages
			std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
			for (auto& [stage,shader] : m_Specification.Stages) {
				VK_STRUCT(VkPipelineShaderStageCreateInfo, shaderStageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);

				shaderStageInfo.pName = "main";
				shaderStageInfo.stage = Utils::KaidelShaderStageToVkShaderStage(stage);
				shaderStageInfo.module = ((VulkanSingleShader*)shader.Get())->GetModule();
				shaderStages.push_back(shaderStageInfo);
			}
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.stageCount = (uint32_t)shaderStages.size();

			//Dynamic states
			std::vector<VkDynamicState> dynamicStates;
			VK_STRUCT(VkPipelineDynamicStateCreateInfo, dynamicStateInfo, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
			dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

			//Viewport and scissor
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)0.0f;
			viewport.height = (float)0.0f;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			scissor.extent = { 0,0 };

			VK_STRUCT(VkPipelineViewportStateCreateInfo, viewportCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
			viewportCreateInfo.pScissors = &scissor;
			viewportCreateInfo.pViewports = &viewport;
			viewportCreateInfo.viewportCount = 1;
			viewportCreateInfo.scissorCount = 1;
			pipelineInfo.pViewportState = &viewportCreateInfo;

			//Rasterizer
			VK_STRUCT(VkPipelineRasterizationStateCreateInfo, rasterizerCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
			rasterizerCreateInfo.cullMode = Utils::KaidelCullModeToVkCullMode(m_Specification.Culling);
			rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizerCreateInfo.frontFace = m_Specification.FrontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
			rasterizerCreateInfo.lineWidth = m_Specification.LineWidth;
			pipelineInfo.pRasterizationState = &rasterizerCreateInfo;

			//Multi-Sampling
			VK_STRUCT(VkPipelineMultisampleStateCreateInfo, multisampleInfo, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
			multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			pipelineInfo.pMultisampleState = &multisampleInfo;



			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;

			const auto colorAttachmentWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			for (auto& attachment : m_Specification.RenderPass->GetSpecification().OutputImages) {
				VkPipelineColorBlendAttachmentState blendAttachment{};
				blendAttachment.colorWriteMask = attachment.StoreOp == RenderPassImageStoreOp::Store ? colorAttachmentWriteMask : 0;
				colorBlendAttachments.push_back(blendAttachment);
			}

			//Color Blend
			VK_STRUCT(VkPipelineColorBlendStateCreateInfo, colorBlendInfo, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
			colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
			colorBlendInfo.pAttachments = colorBlendAttachments.data();
			colorBlendInfo.attachmentCount = (uint32_t)colorBlendAttachments.size();
			colorBlendInfo.blendConstants[0] = 0.0f;
			colorBlendInfo.blendConstants[1] = 0.0f;
			colorBlendInfo.blendConstants[2] = 0.0f;
			colorBlendInfo.blendConstants[3] = 0.0f;
			pipelineInfo.pColorBlendState = &colorBlendInfo;

			//Pipeline Layout

			VK_STRUCT(VkPipelineLayoutCreateInfo, layoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
			VK_ASSERT(vkCreatePipelineLayout(VK_DEVICE, &layoutInfo, VK_ALLOCATOR_PTR, &m_Layout));
			pipelineInfo.layout = m_Layout;
			
			Ref<VulkanRenderPass> rp = m_Specification.RenderPass;
			//Render Pass
			pipelineInfo.renderPass = rp->GetRenderPass();


			//Dynamic State
			dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
			dynamicStateInfo.pDynamicStates = dynamicStates.data();
			pipelineInfo.pDynamicState = &dynamicStateInfo;

			VK_ASSERT(vkCreateGraphicsPipelines(VK_DEVICE, VK_NULL_HANDLE, 1, &pipelineInfo, VK_ALLOCATOR_PTR, &m_Pipeline));
			m_Finalized = true;

		}
	}
}
