#include "KDpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanRenderPass.h"
#include "VulkanMemory.h"

namespace Kaidel {


	static const uint32_t s_MaxFramebufferSize = 8192;
	namespace Utils {



		struct FramebufferImageCreateResult {
			VkImage Image = VK_NULL_HANDLE;
			VkDeviceMemory Memory = VK_NULL_HANDLE;
			VkImageView ImageView = VK_NULL_HANDLE;
		};

		static FramebufferImageCreateResult CreateFramebufferImage(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags imageUsage,VkImageAspectFlags aspectFlags, const std::vector<uint32_t>& queueIndices) {

			FramebufferImageCreateResult image{};
			ImageSpecification imageSpecs{};
			imageSpecs.ArrayLayerCount = 1;
			imageSpecs.Width = width;
			imageSpecs.Height = height;
			imageSpecs.Depth = 1;
			imageSpecs.Format = format;
			imageSpecs.ImageType = VK_IMAGE_TYPE_2D;
			imageSpecs.ImageUsage = imageUsage;
			imageSpecs.InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageSpecs.LogicalDevice = VK_DEVICE;
			imageSpecs.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			imageSpecs.MipLevels = 1;
			imageSpecs.PhysicalDevice = VK_PHYSICAL_DEVICE;
			imageSpecs.QueueFamilies = queueIndices;
			imageSpecs.SampleCount = VK_SAMPLE_COUNT_1_BIT;
			ImageCreateResult imageCreateResult = Utils::CreateImage(imageSpecs);

			image.Image = imageCreateResult.Image;
			image.Memory = imageCreateResult.AllocatedMemory;


			VK_STRUCT(VkImageViewCreateInfo, imageViewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
			imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			imageViewInfo.format = format;
			imageViewInfo.image = image.Image;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = aspectFlags;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange = subresourceRange;

			VK_ASSERT(vkCreateImageView(VK_DEVICE, &imageViewInfo, VK_ALLOCATOR_PTR, &image.ImageView));

			return image;
		}

	}


	

	namespace Vulkan {
		



		VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specification)
			:m_Specification(specification)
		{

			for (auto& attachment : specification.Attachments.Attachments) {
				if (Utils::IsDepthFormat(attachment.Format)) {
					m_DepthAttachmentSpecification = attachment;
				}
				else {
					m_ColorAttachmentSpecifications.push_back(attachment);
				}
			}

			CreateRenderPass();

			Invalidate();
		}

		VulkanFramebuffer::~VulkanFramebuffer()
		{
		}

		void VulkanFramebuffer::Bind()
		{
		}

		void VulkanFramebuffer::Unbind()
		{
		}

		void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
		{
			vkDeviceWaitIdle(VK_DEVICE);
			if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
			{
				KD_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
				return;
			}
			m_Specification.Width = width;
			m_Specification.Height = height;


			//TODO: only invalidate the one that is used by this frame.
			Invalidate();
		}

		void VulkanFramebuffer::Resample(uint32_t newSampleCount)
		{
		}

		void VulkanFramebuffer::ClearAttachment(uint32_t attachmentIndex, const void* colors)
		{
		}

		void VulkanFramebuffer::ClearDepthAttachment(float value)
		{
		}


		FramebufferAttachmentHandle VulkanFramebuffer::GetAttachmentHandle(uint32_t index) const
		{
			return FramebufferAttachmentHandle();
		}

		FramebufferImageHandle VulkanFramebuffer::GetImageHandle(uint64_t index) const
		{
			return FramebufferImageHandle();
		}

		void VulkanFramebuffer::ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output)
		{
		}

		void VulkanFramebuffer::Invalidate()
		{

			if (m_FramebufferResource.GetResources()[0].Framebuffer) {
				DeleteOldBuffers();
			}

			if (m_Specification.Width == 0 || m_Specification.Height == 0) {
				return;
			}


			for (auto& fb : m_FramebufferResource) {
				std::vector<VkImageView> views;

				//Create color attachments
				for (auto& colorAttachment : m_ColorAttachmentSpecifications) {
					auto framebufferImage = 
						Utils::CreateFramebufferImage(Utils::KaidelTextureFormatToVkFormat(colorAttachment.Format), m_Specification.Width, m_Specification.Height,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT,VK_UNIQUE_INDICES);
					fb.ColorAttachments.push_back({ framebufferImage.Image,framebufferImage.Memory,framebufferImage.ImageView });
					views.push_back(framebufferImage.ImageView);
				}

				uint32_t attachmentCount = (uint32_t)fb.ColorAttachments.size();

				//Create a depth attachment if requested
				if (HasDepthAttachment()) {
					auto depthAttachmentImage = 
						Utils::CreateFramebufferImage(Utils::KaidelTextureFormatToVkFormat(m_DepthAttachmentSpecification.Format), m_Specification.Width, m_Specification.Height,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,VK_IMAGE_ASPECT_DEPTH_BIT, VK_UNIQUE_INDICES);
					fb.DepthAttachment = { depthAttachmentImage.Image,depthAttachmentImage.Memory,depthAttachmentImage.ImageView };
					++attachmentCount;
					views.push_back(depthAttachmentImage.ImageView);
				}

				//Create the framebuffer
				VK_STRUCT(VkFramebufferCreateInfo, framebufferInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
				framebufferInfo.attachmentCount = attachmentCount;
				framebufferInfo.pAttachments = views.data();
				framebufferInfo.flags = attachmentCount == 0 ? VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT : 0;
				framebufferInfo.width = m_Specification.Width;
				framebufferInfo.height = m_Specification.Height;
				framebufferInfo.layers = 1;
				framebufferInfo.renderPass = ((VulkanRenderPass*)m_RenderPass.Get())->GetRenderPass();
				vkCreateFramebuffer(VK_DEVICE, &framebufferInfo, VK_ALLOCATOR_PTR, &fb.Framebuffer);
			}


		}

		void VulkanFramebuffer::DeleteOldBuffers()
		{
			for (auto& fb : m_FramebufferResource) {
				vkDestroyFramebuffer(VK_DEVICE, fb.Framebuffer, VK_ALLOCATOR_PTR);
				if (HasDepthAttachment()) {
					DestroyAttachment(fb.DepthAttachment);
				}
				for (auto& col : fb.ColorAttachments) {
					DestroyAttachment(col);
				}
				fb.ColorAttachments.clear();
				fb.DepthAttachment = FramebufferImage{};
			}
		}

		void VulkanFramebuffer::CreateRenderPass()
		{
			RenderPassSpecification spec{};
			spec.BindingPoint = RenderPassBindPoint::Graphics;

			if (HasDepthAttachment()) {
				spec.OutputDepthAttachment =
					RenderPassAttachmentSpecification(m_DepthAttachmentSpecification.Format, RenderPassImageLoadOp::DontCare,
						RenderPassImageStoreOp::DontCare, RenderPassImageLayout::Undefined, RenderPassImageLayout::Depth, RenderPassImageLayout::Depth);
			}

			for (auto& color : m_ColorAttachmentSpecifications) {
				spec.OutputImages.push_back(RenderPassAttachmentSpecification(color.Format, RenderPassImageLoadOp::Clear,
												RenderPassImageStoreOp::Store, RenderPassImageLayout::Undefined,RenderPassImageLayout::Color,
													m_Specification.SwapChainTarget ? RenderPassImageLayout::Present : RenderPassImageLayout::Color));
			}

			m_RenderPass = RenderPass::Create(spec);
		}

		bool VulkanFramebuffer::HasDepthAttachment()
		{
			return Utils::IsDepthFormat(m_DepthAttachmentSpecification.Format);
		}

		void VulkanFramebuffer::DestroyAttachment(FramebufferImage& image)
		{
			vkDestroyImageView(VK_DEVICE, image.ImageView, VK_ALLOCATOR_PTR);
			vkDestroyImage(VK_DEVICE, image.Image, VK_ALLOCATOR_PTR);
		}


	}
}
