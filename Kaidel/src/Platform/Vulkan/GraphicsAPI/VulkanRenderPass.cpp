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
		
		TextureSamples samples = (TextureSamples)0;

		for (auto& attachment : specs.Colors) {
			
			KD_CORE_ASSERT(samples == (TextureSamples)0 || samples == attachment.Samples);
			samples = attachment.Samples;

			VulkanBackend::AttachmentReference ref{};
			ref.Aspects = VK_IMAGE_ASPECT_COLOR_BIT;
			ref.Attachment = attachments.size();
			ref.Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachments.push_back(Utils::ToAttachment(attachment));
		
			subpass.Colors.push_back(ref);
		}
		
		if (Utils::IsDepthFormat(specs.DepthStencil.AttachmentFormat)) {
			
			KD_CORE_ASSERT(samples == (TextureSamples)0 || samples == specs.DepthStencil.Samples);
			samples = specs.DepthStencil.Samples;
			
			VulkanBackend::AttachmentReference ref{};
			ref.Aspects = VK_IMAGE_ASPECT_DEPTH_BIT;
			ref.Attachment = attachments.size();
			ref.Layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			
			auto attachment = Utils::ToAttachment(specs.DepthStencil);
			attachment.Samples = VK_SAMPLE_COUNT_1_BIT;

			attachments.push_back(attachment);
			
			subpass.DepthStencil = ref;

		}

		if (specs.AutoResolve)
		{
			KD_CORE_ASSERT(samples != TextureSamples::x1);

			m_ResolvesOffset = attachments.size();

			for (uint32_t i = 0; i < specs.Colors.size(); ++i) {
				VulkanBackend::AttachmentReference ref{};
				ref.Aspects = VK_IMAGE_ASPECT_COLOR_BIT;
				ref.Attachment = attachments.size();
				ref.Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VulkanBackend::RenderPassAttachment attachment = attachments[i];
				attachment.Samples = VK_SAMPLE_COUNT_1_BIT;

				attachments.push_back(attachment);

				subpass.Resolves.push_back(ref);
			}
		}

		m_ClearValues.resize(attachments.size());

		std::vector<VulkanBackend::SubpassDependency> dependencies;

		for (uint32_t i = 0; i < specs.Dependencies.size(); ++i) {
			const SubpassDependency& dependency = specs.Dependencies[i];
			
			VulkanBackend::SubpassDependency dep;
			dep.Src = 0;
			dep.Dst = 0;

			dep.SrcStages = Utils::PipelineStagesToVulkanPipelineStageFlags(dependency.SrcStages);
			dep.DstStages = Utils::PipelineStagesToVulkanPipelineStageFlags(dependency.DstStages);

			dep.SrcAccesses = Utils::AccessFlagsToVulkanAccessFlags(dependency.SrcAccess);
			dep.DstAccesses = Utils::AccessFlagsToVulkanAccessFlags(dependency.DstAccess);

			dep.DependencyFlags = Utils::SubpassDependencyFlagsToVulkanDependencyFlags(dependency.Flags);
			
			dependencies.push_back(dep);
		}

		m_RenderPass = VK_CONTEXT.GetBackend()->CreateRenderPass(
			std::initializer_list<VulkanBackend::RenderPassAttachment>(attachments.data(), attachments.data() + attachments.size()),
			{ subpass },
			std::initializer_list<VulkanBackend::SubpassDependency>(dependencies.data(), dependencies.data() + dependencies.size())
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
