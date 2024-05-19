#include "KDpch.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsContext.h"
#include "Kaidel/Renderer/GraphicsAPI/Constants.h"

namespace Kaidel {

	namespace Utils {

		static VkImageLayout KaidelImageLayoutToVKImageLayout(RenderPassImageLayout layout) {

			switch (layout)
			{
			case Kaidel::RenderPassImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
			case Kaidel::RenderPassImageLayout::Color:return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case Kaidel::RenderPassImageLayout::Depth:return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			case Kaidel::RenderPassImageLayout::DepthStencil:return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case Kaidel::RenderPassImageLayout::ReadOnly:return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case Kaidel::RenderPassImageLayout::Present:return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}

			KD_CORE_ASSERT(false, "Unknown layout");
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}

		static VkAttachmentLoadOp KaidelLoadOpToVKLoadOp(RenderPassImageLoadOp loadOp) {
			switch (loadOp)
			{
			case Kaidel::RenderPassImageLoadOp::Clear:return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case Kaidel::RenderPassImageLoadOp::Load:return VK_ATTACHMENT_LOAD_OP_LOAD;
			case Kaidel::RenderPassImageLoadOp::DontCare:return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}
			KD_CORE_ASSERT(false, "Unknown load op");
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}

		static VkAttachmentStoreOp KaidelStoreOpToVKStoreOp(RenderPassImageStoreOp storeOp) {
			switch (storeOp)
			{
			case Kaidel::RenderPassImageStoreOp::None:return VK_ATTACHMENT_STORE_OP_NONE;
			case Kaidel::RenderPassImageStoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
			case Kaidel::RenderPassImageStoreOp::DontCare:return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}


			KD_CORE_ASSERT(false, "Unknown store op");
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}

		static VkPipelineBindPoint KaidelBindPointToVkBindPoint(RenderPassBindPoint bindPoint) {
			switch (bindPoint)
			{
			case Kaidel::RenderPassBindPoint::Graphics:return VK_PIPELINE_BIND_POINT_GRAPHICS;
			case Kaidel::RenderPassBindPoint::Compute:return VK_PIPELINE_BIND_POINT_COMPUTE;
			}
			KD_CORE_ASSERT(false, "Unknown bind point");
			return VK_PIPELINE_BIND_POINT_MAX_ENUM;
		}


		static VkAttachmentDescription MakeAttachmentDescription(const RenderPassAttachmentSpecification& attachment) {
			VkAttachmentDescription desc{};
			desc.format = KaidelTextureFormatToVkFormat(attachment.ImageFormat);
			desc.initialLayout = KaidelImageLayoutToVKImageLayout(attachment.InitialLayout);
			desc.finalLayout = KaidelImageLayoutToVKImageLayout(attachment.FinalLayout);
			desc.loadOp = KaidelLoadOpToVKLoadOp(attachment.LoadOp);
			desc.storeOp = KaidelStoreOpToVKStoreOp(attachment.StoreOp);
			desc.samples = VK_SAMPLE_COUNT_1_BIT;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			return desc;
		}

		static VkAttachmentReference MakeAttachmentReference(uint32_t index,const RenderPassAttachmentSpecification& attachemnt) {
			VkAttachmentReference ref{};
			ref.attachment = index;
			ref.layout = KaidelImageLayoutToVKImageLayout(attachemnt.Layout);
			return ref;
		}


	}



	namespace Vulkan {
		VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specification)
			:m_Specification(specification)
		{
			std::vector<VkAttachmentDescription> attachments{};
			std::vector<VkAttachmentReference> inputRefs{};
			{
				for (auto& attachment : specification.InputImages) {
					VkAttachmentDescription desc = Utils::MakeAttachmentDescription(attachment);
					VkAttachmentReference ref = Utils::MakeAttachmentReference((uint32_t)attachments.size(), attachment);

					attachments.push_back(desc);
					inputRefs.push_back(ref);
				}
			}

			std::vector<VkAttachmentReference> outputRefs{};
			{
				for (auto& attachment : specification.OutputImages) {
					VkAttachmentDescription desc = Utils::MakeAttachmentDescription(attachment);
					VkAttachmentReference ref = Utils::MakeAttachmentReference(attachments.size(), attachment);

					attachments.push_back(desc);
					outputRefs.push_back(ref);
				}
			}

			VkAttachmentDescription depthAttachment{};
			VkAttachmentReference depthRef{};
			bool depthSet = false;

			if (specification.OutputDepthAttachment.ImageFormat != TextureFormat::None) {
				depthAttachment = Utils::MakeAttachmentDescription(specification.OutputDepthAttachment);
				depthRef = Utils::MakeAttachmentReference(0, specification.OutputDepthAttachment);
				depthSet = true;
			}
			


			VkSubpassDescription subpass{};
			
			subpass.inputAttachmentCount = (uint32_t)inputRefs.size();
			subpass.pInputAttachments = inputRefs.data();

			subpass.colorAttachmentCount = (uint32_t)outputRefs.size();
			subpass.pColorAttachments = outputRefs.data();
			
			if (depthSet) {
				subpass.pDepthStencilAttachment = &depthRef;
			}


			subpass.pipelineBindPoint = Utils::KaidelBindPointToVkBindPoint(specification.BindingPoint);

			VK_STRUCT(VkRenderPassCreateInfo, renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
			renderPassInfo.attachmentCount = (uint32_t)attachments.size();
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;

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
