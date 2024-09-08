#include "KDpch.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicsContext.h"

#include "VulkanRenderPass.h"

namespace Kaidel {

	namespace Utils {

		static Texture2DSpecification GetTextureSpecification(
			Format format, bool isDepth, uint32_t width,uint32_t height,TextureSamples samples) {
			Texture2DSpecification info{};
			info.Format = format;
			info.Width = width;
			info.Height = height;
			info.Depth = 1;
			info.Layers = 1;
			info.Mips = 1;
			info.Samples = samples;
			info.IsCpuReadable = false;
			info.IsCube = false;
			info.Layout = isDepth ? ImageLayout::DepthAttachmentOptimal : ImageLayout::ColorAttachmentOptimal;
			info.Swizzles[0] = TextureSwizzle::Red;
			info.Swizzles[1] = TextureSwizzle::Green;
			info.Swizzles[2] = TextureSwizzle::Blue; 
			info.Swizzles[3] = TextureSwizzle::Alpha;
			return info;
		}

		static VulkanFramebufferResources CreateFramebuffer(
			const FramebufferSpecification& specs,
			const std::vector<Texture2DSpecification>& colorSpecs,
			const Texture2DSpecification& depth,bool usesDepth, VkRenderPass renderPass)
		{
			const auto& backend = VK_CONTEXT.GetBackend();

			VulkanFramebufferResources ret{};
			ret.Textures.resize(colorSpecs.size() + (uint32_t)usesDepth);

			std::vector<const VulkanBackend::TextureInfo*> infos;
			
			for (uint32_t i = 0; i < colorSpecs.size(); ++i) {
				const auto& color = colorSpecs[i];

				ret.Textures[i] = 
					CreateRef<VulkanFramebufferTexture>(specs.Width,specs.Height,color.Format,specs.Samples,false);

				infos.push_back(((const VulkanBackend::TextureInfo*)ret.Textures[i]->GetBackendInfo()));
			}

			if (usesDepth) {

				ret.Textures[colorSpecs.size()] =
					CreateRef<VulkanFramebufferTexture>(specs.Width, specs.Height, depth.Format, specs.Samples, true);

				infos.push_back(((const VulkanBackend::TextureInfo*)ret.Textures[colorSpecs.size()]->GetBackendInfo()));
			}

			ret.Framebuffer = backend->CreateFramebuffer(renderPass, infos, specs.Width, specs.Height);

			return ret;
		}

		static void DestroyFramebuffer(VulkanFramebufferResources& resources) {
			const auto& backend = VK_CONTEXT.GetBackend();
			backend->DestroyFramebuffer(resources.Framebuffer);
			resources.Textures = {};
		}
	}


	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specs)
		:m_Specification(specs)
	{
		RegisterAttachments();

		//doesnt have render pass.
		if (!m_Specification.OptRenderPass) {
			m_OptRenderPass = false;
			m_Specification.OptRenderPass = CreateRenderPass();
		}
		else {
			m_OptRenderPass = true;
		}
	
		Invalidate();
	}
	VulkanFramebuffer::~VulkanFramebuffer()
	{
		Destroy();
	}
	void VulkanFramebuffer::Recreate(const FramebufferSpecification& newSpecs)
	{

		Destroy();
		ClearData();
		m_Specification = newSpecs;
		RegisterAttachments();

		//doesnt have render pass.
		if (!m_Specification.OptRenderPass) {
			m_OptRenderPass = false;
			m_Specification.OptRenderPass = CreateRenderPass();
		}
		else {
			m_OptRenderPass = true;
		}

		Invalidate();
	}

	void VulkanFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 && height == 0)
			return;
		if (width == m_Specification.Width && height == m_Specification.Height)
			return;
		Destroy();
		m_Specification.Width = width;
		m_Specification.Height = height;
		
		for (auto& color : m_ColorInfos) {
			color.Width = width;
			color.Height = height;
		}

		Invalidate();
	}
	
	void VulkanFramebuffer::Invalidate()
	{
		m_Framebuffer = Utils::CreateFramebuffer(m_Specification, m_ColorInfos, m_DepthInfo, m_HasDepth,
			((VulkanRenderPass*)m_Specification.OptRenderPass.Get())->GetRenderPass());
	}
	
	Ref<RenderPass> VulkanFramebuffer::CreateRenderPass()
	{
		RenderPassSpecification rpSpec{};
		
		uint64_t colorCount = 0;
		for (auto& color : m_ColorAttachments) {
			RenderPassAttachment attachment = {};
			attachment.Format = color.AttachmentFormat;

			attachment.InitialLayout = ImageLayout::None;
			attachment.FinalLayout = ImageLayout::General;

			attachment.Samples = TextureSamples::x1;
			
			attachment.LoadOp = AttachmentLoadOp::Clear;
			attachment.StoreOp = AttachmentStoreOp::Store;
			
			attachment.StencilLoadOp = AttachmentLoadOp::DontCare;
			attachment.StencilStoreOp = AttachmentStoreOp::DontCare;
			
			rpSpec.Attachments.push_back(attachment);
		}

		colorCount = rpSpec.Attachments.size();

		if (m_HasDepth) {
			RenderPassAttachment attachment = {};
			attachment.Format = m_DepthAttachment.AttachmentFormat;

			attachment.InitialLayout = ImageLayout::None;
			attachment.FinalLayout = ImageLayout::General;

			attachment.Samples = TextureSamples::x1;

			attachment.LoadOp = AttachmentLoadOp::Clear;
			attachment.StoreOp = AttachmentStoreOp::Store;

			attachment.StencilLoadOp = AttachmentLoadOp::DontCare;
			attachment.StencilStoreOp = AttachmentStoreOp::DontCare;

			rpSpec.Attachments.push_back(attachment);
		}

		Subpass subpass{};

		for (uint64_t i = 0; i < colorCount; ++i) {
			AttachmentReference ref{};
			ref.Aspects = AspectMask_Color;
			ref.Attachment = i;
			ref.Layout = ImageLayout::ColorAttachmentOptimal;
			subpass.Colors.push_back(ref);
		}

		if (m_HasDepth) {
			AttachmentReference ref{};
			ref.Aspects = AspectMask_Color;
			ref.Attachment = colorCount;
			ref.Layout = ImageLayout::ColorAttachmentOptimal;
			subpass.DepthStencil = ref;
		}

		rpSpec.Subpasses.push_back(subpass);

		return RenderPass::Create(rpSpec);
	}
	
	void VulkanFramebuffer::ClearData()
	{
		m_ColorAttachments.clear();
		m_DepthAttachment.AttachmentFormat = Format::None;
		m_ColorInfos.clear();
		m_HasDepth = false;
		m_OptRenderPass = false;
	}
	
	void VulkanFramebuffer::RegisterAttachments()
	{
		for (uint32_t i = 0; i < m_Specification.Attachments.size(); ++i) {
			const auto& attachment = m_Specification.Attachments[i];
			if (Utils::IsDepthFormat(attachment.AttachmentFormat)) {
				m_HasDepth = true;
				auto spec = Utils::GetTextureSpecification(
					attachment.AttachmentFormat,
					true, m_Specification.Width, m_Specification.Height, m_Specification.Samples);
				memcpy(&m_DepthInfo, &spec, sizeof(spec));
				m_DepthAttachment = attachment;
			}
			else {
				m_ColorInfos.push_back(Utils::GetTextureSpecification(
					attachment.AttachmentFormat,
					false, m_Specification.Width, m_Specification.Height, m_Specification.Samples
				));

				m_ColorAttachments.push_back(attachment);
			}
		}
	}
	
	void VulkanFramebuffer::Destroy()
	{
		Utils::DestroyFramebuffer(m_Framebuffer);
	}
}
