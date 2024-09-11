#include "KDpch.h"
#include "VulkanRenderPass.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	namespace Utils {
		static VulkanBackend::RenderPassAttachment ToAttachment(const RenderPassAttachment& attachment) {
			VulkanBackend::RenderPassAttachment ret{};
			ret.InitialLayout = ImageLayoutToVulkanImageLayout(attachment.InitialLayout);
			ret.FinalLayout = ImageLayoutToVulkanImageLayout(attachment.FinalLayout);
			ret.Format = FormatToVulkanFormat(attachment.AttachmentFormat);
			ret.LoadOp = AttachmentLoadOpToVulkanAttachmentLoadOp(attachment.LoadOp);
			ret.StoreOp = AttachmentStoreOpToVulkanAttachmentStoreOp(attachment.StoreOp);
			ret.StencilLoadOp = AttachmentLoadOpToVulkanAttachmentLoadOp(attachment.StencilLoadOp);
			ret.StencilStoreOp = AttachmentStoreOpToVulkanAttachmentStoreOp(attachment.StencilStoreOp);
			ret.Samples = (VkSampleCountFlagBits)attachment.Samples;

			return ret;
		}
#if 0
		static VulkanBackend::AttachmentReference ToReference(const AttachmentReference& ref) {
			VulkanBackend::AttachmentReference ret{};
			ret.Aspects = ref.Aspects;
			ret.Attachment = ref.Attachment;
			ret.Layout = ImageLayoutToVulkanImageLayout(ref.Layout);
			
			return ret;
		}
		
		static std::vector<VulkanBackend::AttachmentReference> ToReferences(const std::vector<AttachmentReference> refs) {
			std::vector<VulkanBackend::AttachmentReference> ret;
			ret.resize(refs.size());
		
			for (uint32_t i = 0; i < refs.size(); ++i) {
				ret[i] = ToReference(refs[i]);
			}
		
			return ret;
		}

		static VulkanBackend::Subpass ToSubpass(const Subpass& subpass) {
			VulkanBackend::Subpass ret{};
		
			ret.Colors = ToReferences(subpass.Colors);
			ret.DepthStencil = ToReference(subpass.DepthStencil);
			ret.Inputs = ToReferences(subpass.Inputs);
			ret.Preserves = subpass.Preserves;
			ret.Resolves = ToReferences(subpass.Resolves);
			ret.VRS = ToReference(subpass.VRS);
		
			return ret;
		}

		static VulkanBackend::SubpassDependency ToDependency(const SubpassDependency& dependency) {
			VulkanBackend::SubpassDependency ret{};
			ret.Src = dependency.Src;
			ret.Dst = dependency.Dst;
			ret.SrcAccesses = AccessFlagsToVulkanAccessFlags(dependency.SrcAccesses);
			ret.DstAccesses = AccessFlagsToVulkanAccessFlags(dependency.DstAccesses);
			ret.SrcStages = PipelineStagesToVulkanPipelineStageFlags(dependency.SrcStages);
			ret.DstStages = PipelineStagesToVulkanPipelineStageFlags(dependency.DstStages);
		
			return ret;
		}
#endif
	}
	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& specs)
		:m_Specification(specs)
	{
#if 0 
		std::vector<VulkanBackend::RenderPassAttachment> attachments;
		attachments.resize(specs.Attachments.size());
		for (uint32_t i = 0; i < specs.Attachments.size(); ++i) {
			attachments[i] = Utils::ToAttachment(specs.Attachments[i]);
		}

		std::vector<VulkanBackend::Subpass> subpasses;
		subpasses.resize(specs.Subpasses.size());
		for (uint32_t i = 0; i < specs.Subpasses.size(); ++i) {
			subpasses[i] = Utils::ToSubpass(specs.Subpasses[i]);
		}

		std::vector<VulkanBackend::SubpassDependency> dependencies;
		dependencies.resize(specs.Dependencies.size());
		for (uint32_t i = 0; i < dependencies.size(); ++i) {
			dependencies[i] = Utils::ToDependency(specs.Dependencies[i]);
		}

		m_RenderPass = VK_CONTEXT.GetBackend()->CreateRenderPass(
			std::initializer_list(attachments.data(), attachments.data() + attachments.size()),
			std::initializer_list(subpasses.data(), subpasses.data() + subpasses.size()),
			std::initializer_list(dependencies.data(), dependencies.data() + dependencies.size()));
#else
		std::vector<VulkanBackend::RenderPassAttachment> attachments;
		VulkanBackend::Subpass subpass{};
		
		for (auto& attachment : specs.Colors) {
			VulkanBackend::AttachmentReference ref{};
			ref.Aspects = VK_IMAGE_ASPECT_COLOR_BIT;
			ref.Attachment = attachments.size();
			ref.Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachments.push_back(Utils::ToAttachment(attachment));
		
			subpass.Colors.push_back(ref);
		}
		
		if (Utils::IsDepthFormat(specs.DepthStencil.AttachmentFormat)) {
			VulkanBackend::AttachmentReference ref{};
			ref.Aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
			ref.Attachment = attachments.size();
			ref.Layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

			attachments.push_back(Utils::ToAttachment(specs.DepthStencil));
			
			subpass.DepthStencil = ref;
		}

		m_RenderPass = VK_CONTEXT.GetBackend()->CreateRenderPass(
			std::initializer_list<VulkanBackend::RenderPassAttachment>(attachments.data(), attachments.data() + attachments.size()),
			{ subpass },
			{}
		);
#endif
	}
	VulkanRenderPass::~VulkanRenderPass()
	{
		auto rp = m_RenderPass;
		Application::Get().SubmitToMainThread([rp]() {
			VK_CONTEXT.GetBackend()->DestroyRenderPass(rp);
		});
	}
}
