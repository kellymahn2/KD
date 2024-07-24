#include "KDpch.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {

	namespace Utils {
		struct VertexInputState {

			std::vector<VkVertexInputAttributeDescription> Attributes;
			std::vector<VkVertexInputBindingDescription> Bindings;
			VkPipelineVertexInputStateCreateInfo State;
		};

		static VertexInputState MakeVertexInputState(const VertexInputSpecification& inputSpec) {

			VertexInputState result{};
			result.State.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

			uint32_t bindingIndex = 0;
			uint32_t attributeIndex = 0;


			for (auto& binding : inputSpec.Bindings) {
				uint32_t attributeOffset = 0;
				for (auto& element : binding.Elements) {
					//Dummy
					if (element.InputType == VertexInputType::Dummy) {
						attributeOffset += element.Size;
					}
					//Matrix
					else if (element.InputType == VertexInputType::Mat3 || element.InputType == VertexInputType::Mat4) {
						uint32_t rowCount = element.InputType == VertexInputType::Mat3 ? 3 : 4;
						uint32_t rowSize = rowCount * sizeof(float);
						VkFormat format = element.InputType == VertexInputType::Mat3 ? VK_FORMAT_R32G32B32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
						for (uint32_t i = 0; i < rowCount; ++i) {
							VkVertexInputAttributeDescription attrib{};
							attrib.binding = bindingIndex;
							attrib.format = format;
							attrib.location = attributeIndex;
							attrib.offset = attributeOffset;
							result.Attributes.push_back(attrib);
							attributeOffset += rowSize;
							++attributeIndex;
						}
					}
					//Vectors
					else {
						uint32_t size = VertexInputTypeSize(element.InputType);
						VkVertexInputAttributeDescription attrib{};
						attrib.binding = bindingIndex;
						attrib.format = VertexInputTypeToVulkanFormat(element.InputType);
						attrib.location = attributeIndex;
						attrib.offset = attributeOffset;
						result.Attributes.push_back(attrib);
						attributeOffset += size;
						++attributeIndex;
					}

				}
				VkVertexInputBindingDescription bindingDesc{};
				bindingDesc.binding = bindingIndex;
				switch (binding.InputRate)
				{
				case VertexInputRate::Vertex:bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; break;
				case VertexInputRate::Instance:bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE; break;
				}
				bindingDesc.stride = attributeOffset;
				result.Bindings.push_back(bindingDesc);
			}

			result.State.vertexBindingDescriptionCount = (uint32_t)result.Bindings.size();
			result.State.pVertexBindingDescriptions = result.Bindings.data();
			result.State.vertexAttributeDescriptionCount = (uint32_t)result.Attributes.size();
			result.State.pVertexAttributeDescriptions = result.Attributes.data();
			return result;
		}


		static VkPipelineShaderStageCreateInfo MakeShaderStage(VkShaderModule module, VkShaderStageFlagBits stage) {
			VkPipelineShaderStageCreateInfo info{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			info.module = module;
			info.pName = "main";
			info.stage = stage;
			return info;
		}

		struct GraphicsPipelineCreateResult {
			VkPipelineLayout Layout;
			VkPipeline Pipeline;
		};




		static GraphicsPipelineCreateResult CreateGraphicsPipeline(const GraphicsPipelineSpecification& spec) {

			GraphicsPipelineCreateResult result{};

			VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

			std::vector<VkPipelineShaderStageCreateInfo> shaders;

			std::vector<VkDynamicState> dynamics;
			dynamics.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			dynamics.push_back(VK_DYNAMIC_STATE_SCISSOR);
			//Input assembler
			VkPipelineInputAssemblyStateCreateInfo assemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
			assemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pipelineInfo.pInputAssemblyState = &assemblyInfo;

			//Vertex input
			VertexInputState inputState = MakeVertexInputState(spec.InputSpecification);
			pipelineInfo.pVertexInputState = &inputState.State;

#define SHADER_STAGE_INFO(shader,stage,vector) if(shader){vector.push_back(MakeShaderStage((VkShaderModule)shader->GetRendererID(),stage));}


			//Vertex shader
			SHADER_STAGE_INFO(spec.VertexShader, VK_SHADER_STAGE_VERTEX_BIT, shaders);
			//Tesselation control shader
			SHADER_STAGE_INFO(spec.TesselationControlShader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, shaders);

			//Tesselation evaluation shader
			SHADER_STAGE_INFO(spec.TesselationEvaluationShader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, shaders);

			//Geometry shader
			SHADER_STAGE_INFO(spec.GeometryShader, VK_SHADER_STAGE_GEOMETRY_BIT, shaders);

			//Rasterization state
			VkPipelineRasterizationStateCreateInfo rasterizerInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizerInfo.cullMode = PipelineCullModeToVulkanCullMode(spec.CullMode);
			rasterizerInfo.frontFace = spec.FrontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
			rasterizerInfo.lineWidth = 1.0f;
			rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
			pipelineInfo.pRasterizationState = &rasterizerInfo;

			//Depth/stencil state
			VkPipelineDepthStencilStateCreateInfo depthInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
			//depthInfo

			//Fragment shader
			SHADER_STAGE_INFO(spec.FragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT, shaders);


			std::vector<VkPipelineColorBlendAttachmentState> attachments;
			for (auto& attachmentSpec : spec.UsedRenderPass->GetSpecification().OutputColors) {
				VkPipelineColorBlendAttachmentState state{};
				state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				attachments.push_back(state);
			}

			//Color blend
			VkPipelineColorBlendStateCreateInfo blendInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			blendInfo.attachmentCount = (uint32_t)attachments.size();
			blendInfo.pAttachments = attachments.data();
			blendInfo.blendConstants[0] = 0.0f;
			blendInfo.blendConstants[1] = 0.0f;
			blendInfo.blendConstants[2] = 0.0f;
			blendInfo.blendConstants[3] = 0.0f;
			pipelineInfo.pColorBlendState = &blendInfo;

			//Multi sampling
			VkPipelineMultisampleStateCreateInfo multiSampleInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			multiSampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			pipelineInfo.pMultisampleState = &multiSampleInfo;

			//Dynamic state
			VkPipelineDynamicStateCreateInfo dynamicInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamicInfo.dynamicStateCount = (uint32_t)dynamics.size();
			dynamicInfo.pDynamicStates = dynamics.data();
			pipelineInfo.pDynamicState = &dynamicInfo;

			VkViewport viewport{};
			VkRect2D scissor{};
			VkPipelineViewportStateCreateInfo viewportInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportInfo.viewportCount = 1;
			viewportInfo.pViewports = &viewport;
			viewportInfo.scissorCount = 1;
			viewportInfo.pScissors = &scissor;
			pipelineInfo.pViewportState = &viewportInfo;

			pipelineInfo.stageCount = (uint32_t)shaders.size();
			pipelineInfo.pStages = shaders.data();

			pipelineInfo.subpass = 0;
			pipelineInfo.renderPass = (VkRenderPass)spec.UsedRenderPass->GetRendererID();
#undef SHADER_STAGE_INFO

			std::vector<VkDescriptorSetLayout> setLayouts{};

			//Uniform buffers
			for (auto& binding : spec.UniformBufferInput.UniformBufferBindings) {
				setLayouts.push_back(VK_CONTEXT.GetUniformBufferDescriptorSetLayouts()[binding]);
			}

			//Layout
			VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
			layoutInfo.pSetLayouts = setLayouts.data();
			layoutInfo.setLayoutCount = (uint32_t)setLayouts.size();
			VK_ASSERT(vkCreatePipelineLayout(VK_DEVICE.GetDevice(), &layoutInfo, nullptr, &result.Layout));
			pipelineInfo.layout = result.Layout;

			VK_ASSERT(vkCreateGraphicsPipelines(VK_DEVICE.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &result.Pipeline));

			return result;
		}

	}



	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specification)
		:m_Specification(specification)
	{
		Utils::GraphicsPipelineCreateResult result = Utils::CreateGraphicsPipeline(specification);

		m_Pipeline = result.Pipeline;
		m_Layout = result.Layout;
	}
	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		DestroyPipeline();
	}
	void VulkanGraphicsPipeline::Recreate()
	{
		DestroyPipeline();

		Utils::GraphicsPipelineCreateResult result = Utils::CreateGraphicsPipeline(m_Specification);
		m_Layout = result.Layout;
		m_Pipeline = result.Pipeline;
	}
	void VulkanGraphicsPipeline::DestroyPipeline()
	{
		vkDestroyPipelineLayout(VK_DEVICE.GetDevice(), m_Layout, nullptr);
		vkDestroyPipeline(VK_DEVICE.GetDevice(), m_Pipeline, nullptr);
	}
}
