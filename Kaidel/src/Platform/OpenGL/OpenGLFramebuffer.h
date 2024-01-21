#pragma once

#include "Kaidel/Renderer/Framebuffer.h"

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
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;
		virtual void ClearDepthAttachment(float value) override;
		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override { KD_CORE_ASSERT(index < m_ColorAttachments.size()); return (uint64_t)m_ColorAttachments[index]; }
		virtual void SetDepthAttachmentFromArray(uint32_t attachmentID, uint32_t arrayIndex);
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual void BindColorAttachmentToSlot(uint32_t attachmnetIndex, uint32_t slot)override;
		virtual void BindColorAttachmentToImageSlot(uint32_t attachmnetIndex, uint32_t slot,ImageBindingMode bindingMode)override;


		void ClearAttachment(uint32_t attachmentIndex, const float* colors) override;

	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;

	};

}
