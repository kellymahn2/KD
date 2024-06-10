#include "KDpch.h"
#include "VulkanImage.h"

#include "VulkanCommandBuffer.h"

namespace Kaidel {

	namespace Vulkan {




		VulkanFramebufferImage::~VulkanFramebufferImage()
		{
			VkDevice device = VK_DEVICE;
			vkDestroyImageView(device, m_ImageView, VK_ALLOCATOR_PTR);
			vkFreeMemory(device, m_ImageMemory, VK_ALLOCATOR_PTR);
			vkFreeDescriptorSets(device, m_DescriptorPool, 1, &m_DescriptorSet);
			vkDestroySampler(device, m_Sampler, VK_ALLOCATOR_PTR);
			vkDestroyImage(device, m_Image, VK_ALLOCATOR_PTR);
		}

		void VulkanFramebufferImage::TransitionLayout(ImageLayout finalLayout)
		{
			VulkanCommandBuffer commandBuffer = VulkanCommandBuffer(VK_CONTEXT.GetGraphicsCommandPool());

			VkImageLayout oldLayout = GetVulkanImageLayout(m_CurrentLayout);
			VkImageLayout newLayout = GetVulkanImageLayout(finalLayout);

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_Image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else {
				throw std::invalid_argument("unsupported layout transition!");
			}

			vkCmdPipelineBarrier(
				commandBuffer.GetCommandBuffer(),
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			commandBuffer.Submit(CommandBufferSubmitSpecification(VK_DEVICE_QUEUE("GraphicsQueue")));
			vkQueueWaitIdle(VK_DEVICE_QUEUE("GraphicsQueue"));

			m_CurrentLayout = finalLayout;
		}
		VkImageLayout VulkanFramebufferImage::GetVulkanImageLayout(ImageLayout layout)
		{
			switch (layout)
			{
			case Kaidel::ImageLayout::Undefined:return VK_IMAGE_LAYOUT_UNDEFINED;
			case Kaidel::ImageLayout::General:return VK_IMAGE_LAYOUT_GENERAL;
			case Kaidel::ImageLayout::ColorOptimal:return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case Kaidel::ImageLayout::DepthStencilOptimal:return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case Kaidel::ImageLayout::DepthStencilReadOptimal:return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			case Kaidel::ImageLayout::ShaderReadOptimal:return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case Kaidel::ImageLayout::TransferSrc:return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case Kaidel::ImageLayout::TransferDst:return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case Kaidel::ImageLayout::DepthOptimal:return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			case Kaidel::ImageLayout::DepthReadOptimal:return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			case Kaidel::ImageLayout::StencilOptimal:return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
			case Kaidel::ImageLayout::StencilReadOptimal:return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
			case Kaidel::ImageLayout::ReadOptimal:return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
			case Kaidel::ImageLayout::AttachmentOptimal:return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			case Kaidel::ImageLayout::PresentSrc:return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			KD_CORE_ASSERT(false, "Unknown layout");
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}

		

		ImageLayout VulkanFramebufferImage::GetKaidelImageLayout(VkImageLayout layout)
		{
			switch (layout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:return ImageLayout::Undefined;
			case VK_IMAGE_LAYOUT_GENERAL:return ImageLayout::General;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:return ImageLayout::ColorOptimal;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:return ImageLayout::DepthStencilOptimal;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:return ImageLayout::DepthStencilReadOptimal;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:return ImageLayout::ShaderReadOptimal;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:return ImageLayout::TransferSrc;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:return ImageLayout::TransferDst;
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:return ImageLayout::DepthOptimal;
			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:return ImageLayout::DepthReadOptimal;
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:return ImageLayout::StencilOptimal;
			case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:return ImageLayout::StencilReadOptimal;
			case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:return ImageLayout::ReadOptimal;
			case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:return ImageLayout::AttachmentOptimal;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:return ImageLayout::PresentSrc;
			}

			KD_CORE_ASSERT(false, "Unknown layout");
			return ImageLayout::Undefined;
		}
	}
}

