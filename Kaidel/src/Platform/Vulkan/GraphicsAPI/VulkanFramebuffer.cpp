#include "KDpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicsContext.h"
#include "VulkanRenderPass.h"
#include "VulkanMemory.h"
#include "VulkanCommandBuffer.h"

#include "Kaidel/Renderer/RenderCommand.h"
namespace Kaidel {


	static const uint32_t s_MaxFramebufferSize = 8192;
	namespace Utils {



		struct FramebufferImageCreateResult {
			VkImage Image = VK_NULL_HANDLE;
			VkDeviceMemory Memory = VK_NULL_HANDLE;
			VkImageView ImageView = VK_NULL_HANDLE;
			VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
			VkSampler Sampler = VK_NULL_HANDLE;
		};

		struct FramebufferImageSpecification {
			VkFormat Format;
			uint32_t Width, Height;
			VkImageUsageFlags ImageUsage;
			VkImageAspectFlags AspectFlags;
			std::vector<uint32_t> QueueIndices;
			VkDescriptorPool DescriptorPool;
			VkDescriptorSetLayout Layout;
			VkImageLayout FinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		};

		static void TransitionLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
			Vulkan::VulkanCommandBuffer commandBuffer = Vulkan::VulkanCommandBuffer(VK_CONTEXT.GetGraphicsCommandPool());
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
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

			commandBuffer.BeginRecording(0);
			vkCmdPipelineBarrier(
				commandBuffer.GetCommandBuffer(),
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			commandBuffer.EndRecording();

			commandBuffer.Submit(Vulkan::CommandBufferSubmitSpecification(VK_DEVICE_QUEUE("GraphicsQueue")));
		}

		static FramebufferImageCreateResult CreateFramebufferImage(const FramebufferImageSpecification& specs) 
		{
			FramebufferImageCreateResult image{};
			ImageSpecification imageSpecs{};
			imageSpecs.ArrayLayerCount = 1;
			imageSpecs.Width = specs.Width;
			imageSpecs.Height = specs.Height;
			imageSpecs.Depth = 1;
			imageSpecs.Format = specs.Format;
			imageSpecs.ImageType = VK_IMAGE_TYPE_2D;
			imageSpecs.ImageUsage = specs.ImageUsage;
			imageSpecs.InitialLayout = specs.FinalLayout;
			imageSpecs.LogicalDevice = VK_DEVICE;
			imageSpecs.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			imageSpecs.MipLevels = 1;
			imageSpecs.PhysicalDevice = VK_PHYSICAL_DEVICE;
			imageSpecs.QueueFamilies = specs.QueueIndices;
			imageSpecs.SampleCount = VK_SAMPLE_COUNT_1_BIT;
			ImageCreateResult imageCreateResult = Utils::CreateImage(imageSpecs);

			image.Image = imageCreateResult.Image;
			image.Memory = imageCreateResult.AllocatedMemory;


			VK_STRUCT(VkImageViewCreateInfo, imageViewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
			imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			imageViewInfo.format = specs.Format;
			imageViewInfo.image = image.Image;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = specs.AspectFlags;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange = subresourceRange;

			VK_ASSERT(vkCreateImageView(VK_DEVICE, &imageViewInfo, VK_ALLOCATOR_PTR, &image.ImageView));


			VK_STRUCT(VkSamplerCreateInfo, samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 1;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			VK_ASSERT(vkCreateSampler(VK_DEVICE, &samplerInfo, VK_ALLOCATOR_PTR, &image.Sampler));

			VK_STRUCT(VkDescriptorSetAllocateInfo, setInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			setInfo.descriptorPool = specs.DescriptorPool;
			setInfo.descriptorSetCount = 1;
			setInfo.pSetLayouts = &specs.Layout;

			VK_ASSERT(vkAllocateDescriptorSets(VK_DEVICE, &setInfo, &image.DescriptorSet));

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = imageInfo.imageLayout;
			imageInfo.imageView = image.ImageView;
			imageInfo.sampler = image.Sampler;
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = image.DescriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(VK_DEVICE, 1, &descriptorWrite, 0, nullptr);

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
			CreateDescriptorPool();
			CreateDescriptorSetLayout();



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
			//vkDeviceWaitIdle(VK_DEVICE);
			RenderCommand::Submit([this,width,height]() {
				//if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
				//{
				//	KD_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
				//	return;
				//}

				//m_FramebufferResource->FramebufferWidth = width;
				//m_FramebufferResource->FramebufferHeight = height;

				//Invalidate(*m_FramebufferResource);
				
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

			
			if (m_Specification.Width == 0 || m_Specification.Height == 0) {
				return;
			}


			for (auto& fb : m_FramebufferResource) {
				fb.FramebufferWidth = m_Specification.Width;
				fb.FramebufferHeight = m_Specification.Height;
				Invalidate(fb);
			}


		}

		void VulkanFramebuffer::DeleteOldBuffers()
		{
			for (auto& fb : m_FramebufferResource) {
				DeleteFramebufferFrameResource(fb);
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

		void VulkanFramebuffer::CreateDescriptorPool()
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.descriptorCount = m_ColorAttachmentSpecifications.size() + static_cast<uint32_t>(HasDepthAttachment());
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			VK_STRUCT(VkDescriptorPoolCreateInfo, poolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
			poolInfo.maxSets = poolSize.descriptorCount;
			poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			poolInfo.poolSizeCount = 1;
			poolInfo.pPoolSizes = &poolSize;
			VK_ASSERT(vkCreateDescriptorPool(VK_DEVICE, &poolInfo, VK_ALLOCATOR_PTR, &m_DescriptorPool));
		}

		void VulkanFramebuffer::CreateDescriptorSetLayout()
		{

			VkDescriptorSetLayoutBinding binding{};
			binding.binding = 0;
			binding.descriptorCount = 1;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
			VK_STRUCT(VkDescriptorSetLayoutCreateInfo, layoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &binding;

			VK_ASSERT(vkCreateDescriptorSetLayout(VK_DEVICE, &layoutInfo, VK_ALLOCATOR_PTR, &m_SetLayout));
		}

		bool VulkanFramebuffer::HasDepthAttachment()
		{
			return Utils::IsDepthFormat(m_DepthAttachmentSpecification.Format);
		}

		void VulkanFramebuffer::DestroyAttachment(FramebufferImage& image)
		{
		}

		void VulkanFramebuffer::Invalidate(FramebufferFrameResource& fb)
		{
			
			if (fb.FramebufferWidth == 0 || fb.FramebufferHeight == 0) {
				return;
			}

			if (fb.Framebuffer) {
				DeleteFramebufferFrameResource(fb);
			}

			std::vector<VkImageView> views;

			//Create color attachments
			for (auto& colorAttachment : m_ColorAttachmentSpecifications) {

				Utils::FramebufferImageSpecification spec{};
				spec.AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
				spec.DescriptorPool = m_DescriptorPool;
				spec.FinalLayout = VK_IMAGE_LAYOUT_GENERAL;
				spec.Format = Utils::KaidelTextureFormatToVkFormat(colorAttachment.Format);
				spec.Width = fb.FramebufferWidth;
				spec.Height = fb.FramebufferHeight;
				spec.ImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				spec.QueueIndices = VK_UNIQUE_INDICES;
				spec.Layout = m_SetLayout;
				auto framebufferImage = Utils::CreateFramebufferImage(spec);

				FramebufferImage image{};
				image.Image = new VulkanFramebufferImage(framebufferImage.Image, framebufferImage.Memory, 
										framebufferImage.ImageView, framebufferImage.Sampler, framebufferImage.DescriptorSet,m_DescriptorPool, ImageLayout::ColorOptimal);
				fb.ColorAttachments.push_back(image);
				views.push_back(framebufferImage.ImageView);
			}

			uint32_t attachmentCount = (uint32_t)fb.ColorAttachments.size();

			//Create a depth attachment if requested
			if (HasDepthAttachment()) {
				Utils::FramebufferImageSpecification spec{};
				spec.AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
				spec.DescriptorPool = m_DescriptorPool;
				spec.FinalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				spec.Format = Utils::KaidelTextureFormatToVkFormat(m_DepthAttachmentSpecification.Format);
				spec.Width = fb.FramebufferWidth;
				spec.Height = fb.FramebufferHeight;
				spec.ImageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				spec.QueueIndices = VK_UNIQUE_INDICES;
				spec.Layout = m_SetLayout;
				auto framebufferImage = Utils::CreateFramebufferImage(spec);

				FramebufferImage image{};
				image.Image = new VulkanFramebufferImage(framebufferImage.Image, framebufferImage.Memory,
					framebufferImage.ImageView, framebufferImage.Sampler, framebufferImage.DescriptorSet, m_DescriptorPool, ImageLayout::DepthOptimal);
				fb.DepthAttachment = image;
				++attachmentCount;
				views.push_back(framebufferImage.ImageView);
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

		void VulkanFramebuffer::DeleteFramebufferFrameResource(FramebufferFrameResource& fb)
		{
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
}
