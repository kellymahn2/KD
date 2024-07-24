#include "KDpch.h"

#include "VulkanRenderPass.h"
#include "VulkanGraphicsContext.h"


namespace Kaidel 
{
	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification)
		:m_Specification(specification)
	{

		std::vector<VkAttachmentDescription> descs;
		std::vector<VkAttachmentReference> references;
		for (auto& outputColor : m_Specification.OutputColors) 
		{

			{
				VkAttachmentDescription attachment{};
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment.initialLayout = Utils::ImageLayoutToVulkanImageLayout(outputColor.InitialLayout);
				attachment.finalLayout = Utils::ImageLayoutToVulkanImageLayout(outputColor.FinalLayout);
				attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.format = Utils::FormatToVulkanFormat(outputColor.AttachmentFormat);
				descs.push_back(attachment);
			}

			{
				VkAttachmentReference ref{};
				ref.attachment = descs.size() - 1;
				ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				references.push_back(ref);
			}
		}
		

		VkAttachmentReference depthRef{};

		VkSubpassDescription subpassDesc{};
		subpassDesc.colorAttachmentCount = (uint32_t)references.size();
		subpassDesc.pColorAttachments = references.data();

		if (Utils::IsDepthFormat(m_Specification.OutputDepth.AttachmentFormat)) 
		{
			{
				VkAttachmentDescription attachment{};
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
				attachment.initialLayout = Utils::ImageLayoutToVulkanImageLayout(m_Specification.OutputDepth.InitialLayout);
				attachment.finalLayout = Utils::ImageLayoutToVulkanImageLayout(m_Specification.OutputDepth.FinalLayout);
				attachment.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.format = Utils::FormatToVulkanFormat(m_Specification.OutputDepth.AttachmentFormat);
				descs.push_back(attachment);
			}

			{
				depthRef.attachment = descs.size() - 1;
				depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				subpassDesc.pDepthStencilAttachment = &depthRef;
			}
		}

		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkRenderPassCreateInfo passInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		passInfo.attachmentCount = (uint32_t)descs.size();
		passInfo.pAttachments = descs.data();
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpassDesc;

		VK_ASSERT(vkCreateRenderPass(VK_DEVICE.GetDevice(), &passInfo, nullptr, &m_RenderPass));

		m_ClearValues.resize(specification.OutputColors.size() + (uint64_t)Utils::IsDepthFormat(specification.OutputDepth.AttachmentFormat));
	}
	VulkanRenderPass::~VulkanRenderPass()
	{
		vkDestroyRenderPass(VK_DEVICE.GetDevice(), m_RenderPass, nullptr);
	}

	void VulkanRenderPass::SetClearValue(uint32_t attachmentIndex, const AttachmentClearValue& clearValue)
	{

		auto& attachment = m_Specification.OutputColors[attachmentIndex];

		auto& value = m_ClearValues[attachmentIndex];

		if (Utils::IsDepthFormat(attachment.AttachmentFormat)) 
		{
			value.depthStencil.depth = clearValue.DepthStencilClear.Depth;
			value.depthStencil.stencil = clearValue.DepthStencilClear.Stencil;
		}
		else 
		{
			value.color.float32[0] = clearValue.ColorClear.RGBAF.r;
			value.color.float32[1] = clearValue.ColorClear.RGBAF.g;
			value.color.float32[2] = clearValue.ColorClear.RGBAF.b;
			value.color.float32[3] = clearValue.ColorClear.RGBAF.a;
		}
	}
	AttachmentClearValue VulkanRenderPass::GetClearValue(uint32_t attachmentIndex) const
	{
		AttachmentClearValue value{};
		auto& attachment = m_Specification.OutputColors[attachmentIndex];

		if (Utils::IsDepthFormat(attachment.AttachmentFormat)) 
		{
			value.DepthStencilClear.Depth = m_ClearValues[attachmentIndex].depthStencil.depth;
			value.DepthStencilClear.Stencil = m_ClearValues[attachmentIndex].depthStencil.stencil;
		}
		else 
		{
			auto clearValue = m_ClearValues[attachmentIndex].color.float32;
			value.ColorClear.RGBAF = { clearValue[0],clearValue[1] ,clearValue[2] ,clearValue[3] };
		}
		return value;
	}
}
