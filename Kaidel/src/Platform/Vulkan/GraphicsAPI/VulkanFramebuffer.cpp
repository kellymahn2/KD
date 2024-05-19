#include "KDpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanRenderPass.h"

namespace Kaidel {


	namespace Utils {



		struct FramebufferImageCreateResult {
			VkImage Image = VK_NULL_HANDLE;
			VkImageView ImageView = VK_NULL_HANDLE;
		};

		static FramebufferImageCreateResult CreateFramebufferImage(VkFormat format, uint32_t width, uint32_t height, bool isDepth,const uint32_t* queueIndices,uint32_t queueIndexCount) {

			VK_STRUCT(VkImageCreateInfo, imageInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
			imageInfo.arrayLayers = 1;
			imageInfo.extent = { width,height,1};
			imageInfo.format = format;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.mipLevels = 1;
			imageInfo.pQueueFamilyIndices = queueIndices;
			imageInfo.queueFamilyIndexCount = queueIndexCount;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = queueIndexCount == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = isDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			FramebufferImageCreateResult image{};

			VK_ASSERT(vkCreateImage(VK_DEVICE, &imageInfo, VK_ALLOCATOR_PTR, &image.Image));

			VK_STRUCT(VkImageViewCreateInfo, imageViewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
			imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			imageViewInfo.format = format;
			imageViewInfo.image = image.Image;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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

		uint64_t VulkanFramebuffer::GetColorAttachmentRendererID(uint32_t index) const
		{
			return 0;
		}

		FramebufferAttachmentHandle VulkanFramebuffer::GetAttachmentHandle(uint32_t index) const
		{
			return FramebufferAttachmentHandle();
		}

		FramebufferImageHandle VulkanFramebuffer::GetImageHandle(uint64_t index) const
		{
			return FramebufferImageHandle();
		}

		void VulkanFramebuffer::BindColorAttachmentToSlot(uint32_t attachmentIndex, uint32_t slot)
		{
		}

		void VulkanFramebuffer::BindColorAttachmentToImageSlot(uint32_t attachmnetIndex, uint32_t slot, ImageBindingMode bindingMode)
		{
		}

		void VulkanFramebuffer::BindDepthAttachmentToSlot(uint32_t slot)
		{
		}

		void VulkanFramebuffer::CopyColorAttachment(uint32_t dstAttachmentIndex, uint32_t srcAttachmentIndex, Ref<Framebuffer> src)
		{
		}

		void VulkanFramebuffer::CopyDepthAttachment(Ref<Framebuffer> src)
		{
		}

		void VulkanFramebuffer::EnableColorAttachment(uint32_t attachmentIndex)
		{
		}

		void VulkanFramebuffer::DisableColorAttachment(uint32_t attachmentIndex)
		{
		}

		void VulkanFramebuffer::ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output)
		{
		}

		void VulkanFramebuffer::SetAttachment(const TextureHandle& handle, uint32_t index)
		{
		}

		void VulkanFramebuffer::SetAttachment(const TextureArrayHandle& handle, uint32_t index)
		{
		}

		void VulkanFramebuffer::SetDepthAttachment(const TextureHandle& handle)
		{
		}

		void VulkanFramebuffer::SetDepthAttachment(const TextureArrayHandle& handle)
		{
		}

		void VulkanFramebuffer::Invalidate()
		{
			if (m_Framebuffer) {
				//TODO: Destroy old framebuffer
			}

			m_ColorAttachments.clear();
			m_ColorAttachmentSpecifications.clear();
			m_DepthAttachment = {};
			m_Framebuffer = VK_NULL_HANDLE;

			if (m_Specification.Width == 0 || m_Specification.Height == 0) {
				return;
			}


			

			std::vector<VkImageView> views;

			//Create color attachments
			for (auto& colorAttachment : m_ColorAttachmentSpecifications) {
				auto framebufferImage = Utils::CreateFramebufferImage(Utils::KaidelTextureFormatToVkFormat(colorAttachment.Format), m_Specification.Width, m_Specification.Height,false
																		, VK_UNIQUE_INDICES.data(), (uint32_t)VK_UNIQUE_INDICES.size());
				m_ColorAttachments.push_back({ framebufferImage.Image,framebufferImage.ImageView });
				views.push_back(framebufferImage.ImageView);
			}


			uint32_t attachmentCount = (uint32_t)m_ColorAttachments.size();


			//Create a depth attachment if requested
			if (m_DepthAttachmentSpecification.Format != TextureFormat::None) {
				auto depthAttachmentImage = Utils::CreateFramebufferImage(Utils::KaidelTextureFormatToVkFormat(m_DepthAttachmentSpecification.Format), m_Specification.Width, m_Specification.Height, true
																			, VK_UNIQUE_INDICES.data(), (uint32_t)VK_UNIQUE_INDICES.size());
				m_DepthAttachment = { depthAttachmentImage.Image,depthAttachmentImage.ImageView };
				++attachmentCount;
				views.push_back(depthAttachmentImage.ImageView);

			}



			//Create the framebuffer
			VK_STRUCT(VkFramebufferCreateInfo, framebufferInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);

			framebufferInfo.attachmentCount = attachmentCount;
			framebufferInfo.pAttachments = views.data();
			framebufferInfo.flags = attachmentCount > 0 ? 0 : VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT;
			framebufferInfo.width = m_Specification.Width;
			framebufferInfo.height = m_Specification.Height;
			framebufferInfo.layers = 1;
			framebufferInfo.renderPass = reinterpret_cast<const VulkanRenderPass*>(m_Specification.RenderPass)->GetRenderPass();

			VK_ASSERT(vkCreateFramebuffer(VK_DEVICE, &framebufferInfo, VK_ALLOCATOR_PTR, &m_Framebuffer));
		}

	}
}
