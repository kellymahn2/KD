#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "PerFrameResource.h"
#include "Platform/Vulkan/VulkanDefinitions.h"

namespace Kaidel {

	class VulkanFramebuffer : public Framebuffer {
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		~VulkanFramebuffer();
		void Bind() override;

		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;

		void Resample(uint32_t newSampleCount) override;

		void ClearAttachment(uint32_t attachmentIndex, const void* colors) override;

		void ClearDepthAttachment(float value) override;

		const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		uint32_t GetWidth() const override { return m_Specification.Width; }

		uint32_t GetHeight() const override { return m_Specification.Height; }

		Ref<Image> GetImage(uint32_t index) override { return m_Resources->Colors[index].Attachment; }

		Ref<RenderPass> GetDefaultRenderPass() const override { return m_RenderPass; }

		void ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) override;

		RendererID GetRendererID()const override { return (RendererID)m_Resources->Framebuffer; }

		virtual bool HasDepthStencilAttachment()const override{
			return Utils::IsDepthFormat(m_DepthAttachmentSpecification.AttachmentFormat);
		}
		virtual uint32_t GetColorAttachmentCount()const override{
			return m_ColorAttachmentSpecifications.size();
		}
		virtual Ref<Image> GetDepthStencilAttachment() override {
			return m_Resources->Depth.Attachment;
		}


	private:
		struct FramebufferAttachmentResource {
			Ref<Image> Attachment;
			VkSampler Sampler;
			VkImageView View;
		};
		struct FramebufferResources {
			VkFramebuffer Framebuffer;
			std::vector<FramebufferAttachmentResource> Colors;
			FramebufferAttachmentResource Depth;
		};
	private:
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();
		void InvalidateOne(FramebufferResources& fb);
		void DestroyOne(FramebufferResources& fb);
		void DestroyAttachmentResource(FramebufferAttachmentResource& resource);
		void Invalidate();
	private:

		PerFrameResource<FramebufferResources> m_Resources;
		FramebufferSpecification m_Specification;
		Ref<RenderPass> m_RenderPass;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification;


		static inline VkDescriptorSetLayout s_SetLayout;

		friend class RenderCommand;
	};


}
