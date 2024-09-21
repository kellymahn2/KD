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
			const FramebufferSpecification& specs,bool usesDepth)
		{
			const auto& backend = VK_CONTEXT.GetBackend();

			auto& rpSpec = specs.RenderPass->GetSpecification();

			VulkanFramebufferResources ret{};
			ret.Textures.resize(rpSpec.Colors.size() + (uint32_t)usesDepth);

			std::vector<const VulkanBackend::TextureInfo*> infos;
			
			for (uint32_t i = 0; i < rpSpec.Colors.size(); ++i) {
				const auto& color = rpSpec.Colors[i];

				ret.Textures[i] = 
					CreateRef<VulkanFramebufferTexture>(specs.Width,specs.Height,color.AttachmentFormat,color.Samples,false);

				infos.push_back(((const VulkanBackend::TextureInfo*)ret.Textures[i]->GetBackendInfo()));
			}

			if (usesDepth) {

				auto& depth = rpSpec.DepthStencil;

				ret.Textures[rpSpec.Colors.size()] =
					CreateRef<VulkanFramebufferTexture>(specs.Width, specs.Height, depth.AttachmentFormat, depth.Samples, true);

				infos.push_back(((const VulkanBackend::TextureInfo*)ret.Textures[rpSpec.Colors.size()]->GetBackendInfo()));
			}

			ret.Framebuffer = 
				backend->CreateFramebuffer(((VulkanRenderPass*)specs.RenderPass.Get())->GetRenderPass(), infos, specs.Width, specs.Height);

			return ret;
		}

		static void DestroyFramebuffer(VulkanFramebufferResources& resources) {
			const auto& backend = VK_CONTEXT.GetBackend();
			backend->DestroyFramebuffer(resources.Framebuffer);
			resources.Textures.clear();
		}
	}


	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& specs)
		:m_Specification(specs)
	{
		RegisterAttachments();

		Invalidate();
	}
	VulkanFramebuffer::~VulkanFramebuffer()
	{
		const auto& backend = VK_CONTEXT.GetBackend();
		backend->DestroyFramebuffer(m_Framebuffer.Framebuffer);
		m_Framebuffer.Textures.clear();
		m_ColorInfos.clear();
		delete m_DepthInfo;
		m_DepthInfo = nullptr;
	}
	void VulkanFramebuffer::Recreate(const FramebufferSpecification& newSpecs)
	{

		Destroy();
		ClearData();
		m_Specification = newSpecs;
		RegisterAttachments();


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
		m_Framebuffer = Utils::CreateFramebuffer(m_Specification,m_HasDepth);
	}
	
	void VulkanFramebuffer::ClearData()
	{
		m_ColorInfos.clear();
		m_HasDepth = false;
		delete m_DepthInfo;
		m_DepthInfo = nullptr;
	}
	
	void VulkanFramebuffer::RegisterAttachments()
	{
		KD_CORE_ASSERT(m_Specification.RenderPass);
		for (auto& attachment : m_Specification.RenderPass->GetSpecification().Colors) {
			m_ColorInfos.push_back(Utils::GetTextureSpecification(
				attachment.AttachmentFormat,
				false, m_Specification.Width, m_Specification.Height, attachment.Samples
			));
		}

		if (Utils::IsDepthFormat(m_Specification.RenderPass->GetSpecification().DepthStencil.AttachmentFormat)) {
			auto& attachment = m_Specification.RenderPass->GetSpecification().DepthStencil;
			m_HasDepth = true;
			m_DepthInfo = new Texture2DSpecification(Utils::GetTextureSpecification(
				attachment.AttachmentFormat,
				true, m_Specification.Width, m_Specification.Height, attachment.Samples));
		}
	}
	
	void VulkanFramebuffer::Destroy()
	{
		Utils::DestroyFramebuffer(m_Framebuffer);
	}
}
