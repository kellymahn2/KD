#include "KDpch.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanGraphicsContext.h"

#include "VulkanShader.h"
#include "VulkanRenderPass.h"

namespace Kaidel {
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineSpecification& specs)
		:m_Specification(specs)
	{
		const auto& shaderInfo = ((VulkanShader*)specs.Shader.Get())->GetShaderInfo();

		//Vertex
		std::vector<VkVertexInputBindingDescription> inputBindings{};
		std::vector<VkVertexInputAttributeDescription> attributeDescs{};
		{
			uint32_t bindingIndex = 0;
			uint32_t attributeIndex = 0;
			for (auto& binding : specs.Input.Bindings)
			{
				uint32_t attributeOffset = 0;
				for (auto& element : binding.Elements)
				{
					if (element.IsDummy)
					{
						attributeOffset += element.Size;
					}
					else
					{
						uint32_t size = Utils::CalculatePixelSize(element.AttribFormat);
						VkVertexInputAttributeDescription attrib{};
						attrib.binding = bindingIndex;
						attrib.format = Utils::FormatToVulkanFormat(element.AttribFormat);
						attrib.location = attributeIndex;
						attrib.offset = attributeOffset;
						attributeDescs.push_back(attrib);
						attributeOffset += size;
						++attributeIndex;
					}
				}

				VkVertexInputBindingDescription bindingDesc{};
				bindingDesc.binding = bindingIndex;
				bindingDesc.inputRate = binding.InputRate == VertexInputRate::Vertex ? VK_VERTEX_INPUT_RATE_VERTEX :
					binding.InputRate == VertexInputRate::Instance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_MAX_ENUM;
				bindingDesc.stride = attributeOffset;

				++bindingIndex;

				inputBindings.push_back(bindingDesc);
			}
		}
		VkPipelineVertexInputStateCreateInfo vertexInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
		vertexInputState.pVertexBindingDescriptions = inputBindings.data();
		vertexInputState.vertexBindingDescriptionCount = (uint32_t)inputBindings.size();
		vertexInputState.pVertexAttributeDescriptions = attributeDescs.data();
		vertexInputState.vertexAttributeDescriptionCount = (uint32_t)attributeDescs.size();

		//Tesselation
		VkPipelineTessellationStateCreateInfo tesselationState{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
		tesselationState.patchControlPoints = specs.Rasterization.PatchControlPoints;

		//Rasterization
		VkPipelineRasterizationStateCreateInfo rasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
		rasterizationState.depthClampEnable = specs.Rasterization.DepthClamp;
		rasterizationState.rasterizerDiscardEnable = specs.Rasterization.DiscardPrimitives;
		rasterizationState.polygonMode = specs.Rasterization.Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = Utils::PipelineCullModeToVulkanCullMode(specs.Rasterization.CullMode);
		rasterizationState.frontFace = specs.Rasterization.FrontCCW ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
		rasterizationState.depthBiasEnable = specs.Rasterization.DepthBias;
		rasterizationState.depthBiasConstantFactor = specs.Rasterization.DepthBiasConstant;
		rasterizationState.depthBiasClamp = specs.Rasterization.DepthBiasClamp;
		rasterizationState.depthBiasSlopeFactor = specs.Rasterization.DepthBiasSlope;
		rasterizationState.lineWidth = specs.Rasterization.LineWidth;

		//Multisample
		VkPipelineMultisampleStateCreateInfo multisampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
		multisampleState.rasterizationSamples = (VkSampleCountFlagBits)specs.Multisample.Samples;
		multisampleState.sampleShadingEnable = specs.Multisample.SampleShading;
		multisampleState.minSampleShading = specs.Multisample.SampleShadingMin;
		multisampleState.pSampleMask = specs.Multisample.SampleMasks.data();
		multisampleState.alphaToCoverageEnable = specs.Multisample.AlphaToCoverage;
		multisampleState.alphaToOneEnable = specs.Multisample.AlphaToOne;

		//Depth Stencil
		VkPipelineDepthStencilStateCreateInfo depthState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
		depthState.depthTestEnable = specs.DepthStencil.DepthTest;
		depthState.depthWriteEnable = specs.DepthStencil.DepthWrite;
		depthState.depthCompareOp = Utils::CompareOpToVulkanCompareOp(specs.DepthStencil.DepthCompareOperator);
		depthState.depthBoundsTestEnable = specs.DepthStencil.DepthRange;
		depthState.stencilTestEnable = specs.DepthStencil.Stencil;

		depthState.front.failOp = Utils::StencilOpToVulkanStencilOp(specs.DepthStencil.StencilFrontOp.Fail);
		depthState.front.passOp = Utils::StencilOpToVulkanStencilOp(specs.DepthStencil.StencilFrontOp.Pass);
		depthState.front.depthFailOp = Utils::StencilOpToVulkanStencilOp(specs.DepthStencil.StencilFrontOp.DepthFail);
		depthState.front.compareOp = Utils::CompareOpToVulkanCompareOp(specs.DepthStencil.StencilFrontOp.Compare);
		depthState.front.compareMask = specs.DepthStencil.StencilFrontOp.CompareMask;
		depthState.front.writeMask = specs.DepthStencil.StencilFrontOp.WriteMask;
		depthState.front.reference = specs.DepthStencil.StencilFrontOp.Reference;

		depthState.back.failOp = Utils::StencilOpToVulkanStencilOp(specs.DepthStencil.StencilBackOp.Fail);
		depthState.back.passOp = Utils::StencilOpToVulkanStencilOp(specs.DepthStencil.StencilBackOp.Pass);
		depthState.back.depthFailOp = Utils::StencilOpToVulkanStencilOp(specs.DepthStencil.StencilBackOp.DepthFail);
		depthState.back.compareOp = Utils::CompareOpToVulkanCompareOp(specs.DepthStencil.StencilBackOp.Compare);
		depthState.back.compareMask = specs.DepthStencil.StencilBackOp.CompareMask;
		depthState.back.writeMask = specs.DepthStencil.StencilBackOp.WriteMask;
		depthState.back.reference = specs.DepthStencil.StencilBackOp.Reference;

		depthState.minDepthBounds = specs.DepthStencil.DepthRangeMin;
		depthState.maxDepthBounds = specs.DepthStencil.DepthRangeMax;

		//Blend State
		VkPipelineColorBlendStateCreateInfo blendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
		blendState.logicOpEnable = specs.Blend.LogicOpEnable;
		blendState.logicOp = Utils::LogicOpToVulkanLogicOp(specs.Blend.LogicOp);

		std::vector<VkPipelineColorBlendAttachmentState> attachmentStates;
		
		auto rp = (VulkanRenderPass*)specs.RenderPass.Get();
		
		{
			const auto& rpSpec = rp->GetSpecification();
			const auto& subpass = rpSpec.Subpasses[specs.Subpass];
			const auto& subpassColors = subpass.Colors;

			for (uint32_t i = 0; i < subpassColors.size(); ++i)
			{
				VkPipelineColorBlendAttachmentState blendAttachment{};
				if (subpassColors[i].Attachment != -1)
				{
					blendAttachment.blendEnable = specs.Blend.Attachments[i].Blend;

					blendAttachment.srcColorBlendFactor = Utils::BlendFactorToVulkanBlendFactor(specs.Blend.Attachments[i].SrcColorBlend);
					blendAttachment.dstColorBlendFactor = Utils::BlendFactorToVulkanBlendFactor(specs.Blend.Attachments[i].DstColorBlend);
					blendAttachment.colorBlendOp = Utils::BlendOpToVulkanBlendOp(specs.Blend.Attachments[i].ColorBlendOp);

					blendAttachment.srcAlphaBlendFactor = Utils::BlendFactorToVulkanBlendFactor(specs.Blend.Attachments[i].SrcAlphaBlend);
					blendAttachment.dstAlphaBlendFactor = Utils::BlendFactorToVulkanBlendFactor(specs.Blend.Attachments[i].DstAlphaBlend);
					blendAttachment.alphaBlendOp = Utils::BlendOpToVulkanBlendOp(specs.Blend.Attachments[i].AlphaBlendOp);

					if (specs.Blend.Attachments[i].WriteR) {
						blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
					}
					if (specs.Blend.Attachments[i].WriteG) {
						blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
					}
					if (specs.Blend.Attachments[i].WriteB) {
						blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
					}
					if (specs.Blend.Attachments[i].WriteA) {
						blendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
					}
				}

				attachmentStates.push_back(blendAttachment);
			}
		}
		blendState.pAttachments = attachmentStates.data();
		blendState.attachmentCount = (uint32_t)attachmentStates.size();

		blendState.blendConstants[0] = specs.Blend.BlendConstant[0];
		blendState.blendConstants[1] = specs.Blend.BlendConstant[1];
		blendState.blendConstants[2] = specs.Blend.BlendConstant[2];
		blendState.blendConstants[3] = specs.Blend.BlendConstant[3];

		//Dynamic state
		VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
		VkDynamicState dynamic[9] = { VK_DYNAMIC_STATE_MAX_ENUM };
		uint32_t dynamicStateCount = 0;


		dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_VIEWPORT; // Viewport and scissor are always dynamic.
		dynamicStateCount++;
		dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_SCISSOR;
		dynamicStateCount++;

		if (specs.DynamicStates & PipelineDynamicState_LineWidth)
		{
			dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_LINE_WIDTH;
			dynamicStateCount++;
		}
		if (specs.DynamicStates & PipelineDynamicState_DepthBias)
		{
			dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_BIAS;
			dynamicStateCount++;
		}
		if (specs.DynamicStates & PipelineDynamicState_BlendConstants)
		{
			dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
			dynamicStateCount++;
		}
		if (specs.DynamicStates & PipelineDynamicState_DepthBounds)
		{
			dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
			dynamicStateCount++;
		}
		if (specs.DynamicStates & PipelineDynamicState_StencilCompareMask)
		{
			dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
			dynamicStateCount++;
		}
		if (specs.DynamicStates & PipelineDynamicState_StencilWriteMask)
		{
			dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
			dynamicStateCount++;
		}
		if (specs.DynamicStates & PipelineDynamicState_StencilReference)
		{
			dynamic[dynamicStateCount] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
			dynamicStateCount++;
		}

		dynamicState.pDynamicStates = dynamic;
		dynamicState.dynamicStateCount = dynamicStateCount;
		
		m_Pipeline = VK_CONTEXT.GetBackend()->CreateGraphicsPipeline(
			shaderInfo,
			vertexInputState,
			Utils::PrimitiveTopologyToVulkanPrimitiveTopology(specs.Primitive),
			tesselationState,
			rasterizationState,
			multisampleState,
			depthState,
			blendState,
			dynamicState,
			rp->GetRenderPass(),
			specs.Subpass);
	}
	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		VK_CONTEXT.GetBackend()->DestroyGraphicsPipeline(m_Pipeline);
	}
}
