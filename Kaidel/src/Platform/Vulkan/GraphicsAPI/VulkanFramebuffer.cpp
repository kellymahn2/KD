#include "KDpch.h"
#include "VulkanFramebuffer.h"
#include "Kaidel/Core/Application.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {

	namespace Utils {
		static VkImageView CreateImageView(VkDevice device, const Image& image, VkImageAspectFlags aspectFlags) {

			VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.format = FormatToVulkanFormat(image.ImageFormat);
			viewInfo.image = (VkImage)image._InternalImageID;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.aspectMask = aspectFlags;

			VkImageView view;
			VK_ASSERT(vkCreateImageView(device, &viewInfo, nullptr, &view));
			return view;
		}

		static VkSampler CreateSampler(VkDevice device) {
			VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.anisotropyEnable = false;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.unnormalizedCoordinates = false;
			VkSampler sampler;
			vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
			return sampler;
		}

	}


	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& spec)
		:m_Specification(spec)
	{
		for (auto& attachment : m_Specification.Attachments.Attachments) {
			if (Utils::IsDepthFormat(attachment.AttachmentFormat))
				m_DepthAttachmentSpecification = attachment;
			else
				m_ColorAttachmentSpecifications.push_back(attachment);
		}

		CreateRenderPass();

		Invalidate();
	}
	VulkanFramebuffer::~VulkanFramebuffer()
	{
		for (auto& fb : m_Resources) {
			if (fb.Framebuffer) {
				DestroyOne(fb);
			}
		}
	}
	void VulkanFramebuffer::Bind()
	{
	}
	void VulkanFramebuffer::Unbind()
	{
	}
	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;

		Application::Get().SubmitToMainThread([this, width, height]() {

			vkDeviceWaitIdle(VK_DEVICE.GetDevice());

			m_Specification.Width = width;
			m_Specification.Height = height;

			Invalidate();
			});
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
	
	void VulkanFramebuffer::ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output)
	{
	}
	void VulkanFramebuffer::CreateRenderPass()
	{
		RenderPassSpecification spec{};

		for (auto& colorSpec : m_ColorAttachmentSpecifications) {
			AttachmentSpecification attachment{};
			attachment.AttachmentFormat = colorSpec.AttachmentFormat;
			attachment.InitialLayout = ImageLayout::None;
			attachment.FinalLayout = ImageLayout::ColorAttachmentOptimal;
			spec.OutputColors.push_back(attachment);
		}

		if (Utils::IsDepthFormat(m_DepthAttachmentSpecification.AttachmentFormat)) {
			spec.OutputDepth.AttachmentFormat = m_DepthAttachmentSpecification.AttachmentFormat;
			spec.OutputDepth.InitialLayout = ImageLayout::None;
			spec.OutputDepth.FinalLayout = ImageLayout::DepthAttachmentOptimal;
		}

		m_RenderPass = RenderPass::Create(spec);
	}
	
	void VulkanFramebuffer::InvalidateOne(FramebufferResources& fb)
	{
		if (fb.Framebuffer) {
			DestroyOne(fb);
		}

		std::vector<VkImageView> views;

		for (auto& colorSpec : m_ColorAttachmentSpecifications) {
			Image image =
				VK_ALLOCATOR.AllocateImage(m_Specification.Width, m_Specification.Height, 1, 1, m_Specification.Samples,
					1, colorSpec.AttachmentFormat, ImageLayout::None, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_IMAGE_TYPE_2D);

			VkImageView view = Utils::CreateImageView(VK_DEVICE.GetDevice(), image, VK_IMAGE_ASPECT_COLOR_BIT);

			VkSampler sampler = Utils::CreateSampler(VK_DEVICE.GetDevice());
			image.Sampler = (RendererID)sampler;
			image.ImageView = (RendererID)view;

			fb.Colors.push_back({ image,sampler,view });
			views.push_back(view);
		}

		if (Utils::IsDepthFormat(m_DepthAttachmentSpecification.AttachmentFormat)) {
			Image image =
				VK_ALLOCATOR.AllocateImage(m_Specification.Width, m_Specification.Height, 1, 1, m_Specification.Samples,
					1, m_DepthAttachmentSpecification.AttachmentFormat, ImageLayout::None, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_IMAGE_TYPE_2D);
			VkImageView view = Utils::CreateImageView(VK_DEVICE.GetDevice(), image, VK_IMAGE_ASPECT_DEPTH_BIT);

			VkSampler sampler = Utils::CreateSampler(VK_DEVICE.GetDevice());

			fb.Depth = { image,sampler,view };
			views.push_back(view);
		}

		VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.attachmentCount = (uint32_t)views.size();
		framebufferInfo.pAttachments = views.data();
		framebufferInfo.width = m_Specification.Width;
		framebufferInfo.height = m_Specification.Height;
		framebufferInfo.layers = 1;
		framebufferInfo.flags = views.empty() ? VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT : 0;
		framebufferInfo.renderPass = (VkRenderPass)m_RenderPass->GetRendererID();

		VK_ASSERT(vkCreateFramebuffer(VK_DEVICE.GetDevice(), &framebufferInfo, nullptr, &fb.Framebuffer));
	}
	void VulkanFramebuffer::DestroyOne(FramebufferResources& fb)
	{
		vkDestroyFramebuffer(VK_DEVICE.GetDevice(), fb.Framebuffer, nullptr);
		
		for (auto& color : fb.Colors) {
			DestroyAttachmentResource(color);
		}
		if (HasDepthStencilAttachment()) {
			DestroyAttachmentResource(fb.Depth);
		}

		fb.Framebuffer = VK_NULL_HANDLE;
		fb.Colors.clear();
		fb.Depth = {};
	}
	void VulkanFramebuffer::DestroyAttachmentResource(FramebufferAttachmentResource& resource)
	{
		vkDestroySampler(VK_DEVICE.GetDevice(), resource.Sampler, nullptr);
		vkDestroyImageView(VK_DEVICE.GetDevice(), resource.View, nullptr);
		VK_ALLOCATOR.DestroyImage(resource.Attachment);
	}
	void VulkanFramebuffer::Invalidate()
	{
		for (auto& fb : m_Resources) {
			InvalidateOne(fb);
		}
	}
}
