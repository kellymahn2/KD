#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"

namespace Kaidel {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Resample(uint32_t newSampleCount)override;

		virtual void ClearDepthAttachment(float value) override;
		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override { KD_CORE_ASSERT(index < m_ColorAttachments.size()); return (uint64_t)m_ColorAttachments[index]; }


		virtual FramebufferAttachmentHandle GetAttachmentHandle(uint32_t index = 0)const override;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual void BindColorAttachmentToSlot(uint32_t attachmnetIndex, uint32_t slot)override;
		virtual void BindColorAttachmentToImageSlot(uint32_t attachmnetIndex, uint32_t slot,ImageBindingMode bindingMode)override;
		virtual void BindDepthAttachmentToSlot(uint32_t slot)override;

		virtual void CopyColorAttachment(uint32_t dstAttachmentIndex,uint32_t srcAttachmentIndex,Ref<Framebuffer> src) override;
		virtual void CopyDepthAttachment(Ref<Framebuffer> src) override;

		virtual void DisableColorAttachment(uint32_t attachmentIndex) override;
		virtual void EnableColorAttachment(uint32_t attachmentIndex) override;


		virtual void SetAttachment(const TextureHandle& handle, uint32_t index = 0) override;
		virtual void SetAttachment(const TextureArrayHandle& handle, uint32_t index = 0) override;
		virtual void SetDepthAttachment(const TextureHandle& handle) override;
		virtual void SetDepthAttachment(const TextureArrayHandle& handle) override;


		void ClearAttachment(uint32_t attachmentIndex, const void* colors) override;
		virtual void ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) override;

	private:


		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = TextureFormat::None;

		std::vector<uint32_t> m_ColorAttachments;
		std::vector<uint32_t> m_DrawBuffers;
		uint32_t m_DepthAttachment = 0;

		friend class OpenGLTexture2DArray;
		friend class OpenGLTexture2D;
		friend class OpenGLTextureCopier;
	};

}
