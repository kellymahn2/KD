#include "KDpch.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsContext.h"
#include "Kaidel/Renderer/GraphicsAPI/Constants.h"

namespace Kaidel {


	namespace Vulkan {
		VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification)
		{
			std::vector<VkAttachmentReference> colorAttachmentRefs;
			std::vector<VkAttachmentDescription> colorAttachments;
			VkAttachmentReference depthAttachmentRef{};
			VkAttachmentDescription depthAttachment{};
			VkSubpassDescription subpass{};
			bool depthSet = false;


			for (auto& outputImage : specification.OutputImages) {
				{
					colorAttachments.push_back(*outputImage.VK.ImageDescription);
				}
				{
					VkAttachmentReference attachmentRef{};
					attachmentRef.attachment = (uint32_t)(colorAttachments.size() - 1);
					attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					colorAttachmentRefs.push_back(attachmentRef);
				}
			}

			subpass.colorAttachmentCount = (uint32_t)(colorAttachmentRefs.size());
			subpass.inputAttachmentCount = 0;
			subpass.pColorAttachments = colorAttachmentRefs.data();
			subpass.pipelineBindPoint = specification.BindingPoint == RenderPassBindPoint::Graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;


			VK_STRUCT(VkRenderPassCreateInfo, renderPassInfo, RENDER_PASS_CREATE_INFO);
			renderPassInfo.attachmentCount = (uint32_t)colorAttachments.size();
			renderPassInfo.pAttachments = colorAttachments.data();
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.subpassCount = 1;
			VK_ASSERT(vkCreateRenderPass(VK_DEVICE, &renderPassInfo, VK_ALLOCATOR_PTR, &m_RenderPass));

		}
		VulkanRenderPass::~VulkanRenderPass()
		{
			vkDestroyRenderPass(VK_DEVICE, m_RenderPass, VK_ALLOCATOR_PTR);
		}
		void VulkanRenderPass::Begin() const
		{
		}
		void VulkanRenderPass::End() const
		{
		}
	}
}
