#include "KDpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/OpenGL/OpenGLTextureArray.h"
#include "OpenGLTextureFormat.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Kaidel {

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {

		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			BindTexture(multisampled, id);
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, GL_FALSE, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum internalFormat, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			BindTexture(multisampled, id);
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D,0,internalFormat,width,height,GL_FALSE,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(TextureFormat format)
		{
			switch (format)
			{
				case TextureFormat::Depth16:  return true;
				case TextureFormat::Depth32F: return true;
				case TextureFormat::Depth32: return true;
				case TextureFormat::Depth24Stencil8: return true;
			}

			return false;
		}


		static GLenum KaidelImageBindingToGL(ImageBindingMode bindingMode) {
			if (bindingMode & ImageBindingMode_Read) {
				if (bindingMode & ImageBindingMode_Write) {
					return GL_READ_WRITE;
				}
				return GL_READ_ONLY;
			}
			if (bindingMode & ImageBindingMode_Write) {
				return GL_WRITE_ONLY;
			}
			return (GLenum)0;
		}



		


	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto& spec : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.Format))
				m_ColorAttachmentSpecifications.emplace_back(spec);
			else
				m_DepthAttachmentSpecification = spec;
		}
		
		Invalidate();
	}

	void OpenGLFramebuffer::BindColorAttachmentToSlot(uint32_t attachmnetIndex, uint32_t slot) {
		glBindTextureUnit(slot, m_ColorAttachments[attachmnetIndex]);
	}

	void OpenGLFramebuffer::BindColorAttachmentToImageSlot(uint32_t attachmnetIndex, uint32_t slot, ImageBindingMode bindingMode) {
		glBindImageTexture(slot,m_ColorAttachments[attachmnetIndex], 0, GL_FALSE, 0, Utils::KaidelImageBindingToGL(bindingMode), Utils::KaidelTextureFormatToGLInternalFormat(m_ColorAttachmentSpecifications[attachmnetIndex].Format));
	}


	void OpenGLFramebuffer::CopyColorAttachment(uint32_t dstAttachmentIndex, uint32_t srcAttachmentIndex, Ref<Framebuffer> src) {
		auto oglSrc = DynamicPointerCast<OpenGLFramebuffer>(src);
		if (dstAttachmentIndex >= m_Specification.Attachments.Attachments.size() || srcAttachmentIndex >= oglSrc->m_Specification.Attachments.Attachments.size())
			return;
		if (oglSrc->m_Specification.Width != m_Specification.Width || oglSrc->m_Specification.Height != m_Specification.Height)
			return;
		bool multiSampled = m_Specification.Samples > 1;
		bool srcMultiSampled = oglSrc->m_Specification.Samples > 1;
		glCopyImageSubData(oglSrc->m_ColorAttachments[srcAttachmentIndex], Utils::TextureTarget(srcMultiSampled), 0, 0, 0, 0, m_ColorAttachments[dstAttachmentIndex], Utils::TextureTarget(multiSampled), 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, 1);
	}
	void OpenGLFramebuffer::CopyDepthAttachment(Ref<Framebuffer> src) {
		auto oglSrc = DynamicPointerCast<OpenGLFramebuffer>(src);
		if (oglSrc->m_Specification.Width != m_Specification.Width || oglSrc->m_Specification.Height != m_Specification.Height)
			return;
		if (!m_DepthAttachment || !oglSrc->m_DepthAttachment)
			return;
		bool multiSampled = m_Specification.Samples > 1;
		bool srcMultiSampled = oglSrc->m_Specification.Samples > 1;
		glCopyImageSubData(oglSrc->m_DepthAttachment, Utils::TextureTarget(srcMultiSampled), 0, 0, 0, 0, m_DepthAttachment, Utils::TextureTarget(multiSampled), 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, 1);
	}
	


	
	void OpenGLFramebuffer::BindDepthAttachmentToSlot(uint32_t slot) {
		if (!m_DepthAttachment)
			return;
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
	}


	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		if (m_RendererID)
		{
			Unbind();
			if (m_ColorAttachments.size())
				glDeleteTextures((uint32_t)m_ColorAttachments.size(), m_ColorAttachments.data());
			if (m_DepthAttachment)
				glDeleteTextures(1, &m_DepthAttachment);

			glDeleteFramebuffers(1, &m_RendererID);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}
	}

	FramebufferAttachmentHandle OpenGLFramebuffer::GetAttachmentHandle(uint32_t index)const {
		FramebufferAttachmentHandle handle{};
		handle.Framebuffer = const_cast<OpenGLFramebuffer*>(this);
		handle.AttachmentIndex = index;
		return handle;
	}

	FramebufferImageHandle OpenGLFramebuffer::GetImageHandle(uint64_t index) const
	{
		FramebufferImageHandle imageHandle{};
		/*imageHandle.OGL.Framebuffer = m_RendererID;
		imageHandle.OGL.Image = m_ColorAttachments[index];
		imageHandle.OGL.ImageIndexInFramebuffer = index;*/
		return imageHandle;
	}

	void OpenGLFramebuffer::ClearDepthAttachment(float value) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glClearTexImage(m_DepthAttachment, 0, Utils::KaidelTextureFormatToGLFormat(m_DepthAttachmentSpecification.Format), GL_FLOAT, &value);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}
	void OpenGLFramebuffer::DisableColorAttachment(uint32_t attachmentIndex){
		KD_CORE_ASSERT(attachmentIndex < m_DrawBuffers.size());
		m_DrawBuffers[attachmentIndex] = GL_NONE;
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glDrawBuffers((GLsizei)m_DrawBuffers.size(), m_DrawBuffers.data());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void OpenGLFramebuffer::EnableColorAttachment(uint32_t attachmentIndex) {
		KD_CORE_ASSERT(attachmentIndex < m_DrawBuffers.size());
		m_DrawBuffers[attachmentIndex] = GL_COLOR_ATTACHMENT0 + attachmentIndex;
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glDrawBuffers((GLsizei)m_DrawBuffers.size(), m_DrawBuffers.data());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			KD_CORE_WARN("Attempted to resize framebuffer to {0}, {1}", width, height);
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;
		
		Invalidate();
	}
	void OpenGLFramebuffer::Resample(uint32_t newSampleCount) {
		if (newSampleCount == 0) {
			KD_CORE_WARN("Attempted to resample to 0 samples");
			return;
		}
		m_Specification.Samples = newSampleCount;
		Invalidate();
	}
	

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, const void* colors)
	{
		KD_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		auto& i = m_ColorAttachmentSpecifications.at(attachmentIndex);
		
		glClearTexImage(m_ColorAttachments.at(attachmentIndex), 0, Utils::KaidelTextureFormatToGLFormat(i.Format), Utils::KaidelTextureFormatToGLValueFormat(i.Format), colors);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	void OpenGLFramebuffer::ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) {
		KD_CORE_ASSERT(attachemntIndex < m_ColorAttachments.size());
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachemntIndex);

		glReadPixels(x, y, w, h, Utils::KaidelTextureFormatToGLFormat(m_ColorAttachmentSpecifications[attachemntIndex].Format),GL_FLOAT,output);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			Unbind();
			if (m_ColorAttachments.size())
				glDeleteTextures((GLsizei)m_ColorAttachments.size(), m_ColorAttachments.data());
			if (m_DepthAttachment)
				glDeleteTextures(1, &m_DepthAttachment);

			glDeleteFramebuffers(1, &m_RendererID);

			m_ColorAttachments.clear();
			m_DrawBuffers.clear();
			m_DepthAttachment = 0;
			m_RendererID = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);


		bool multisample = m_Specification.Samples > 1;

		// Attachments
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), (uint32_t)m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multisample, m_ColorAttachments[i]);
				Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, Utils::KaidelTextureFormatToGLInternalFormat(m_ColorAttachmentSpecifications[i].Format),
					Utils::KaidelTextureFormatToGLFormat(m_ColorAttachmentSpecifications[i].Format), m_Specification.Width, m_Specification.Height, (int)i);
				m_DrawBuffers.push_back((uint32_t)(GL_COLOR_ATTACHMENT0 + i));
			}
		}

		if (m_DepthAttachmentSpecification.Format != TextureFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, m_DepthAttachment);
			Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, Utils::KaidelTextureFormatToGLInternalFormat(m_DepthAttachmentSpecification.Format), GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
		}

		if (!m_DrawBuffers.empty())
		{
			glDrawBuffers((GLsizei)m_DrawBuffers.size(), m_DrawBuffers.data());
		}
		else {
			glDrawBuffer(GL_NONE);
		}

		KD_CORE_ASSERT((m_DepthAttachmentSpecification.Format == TextureFormat::None && m_DrawBuffers.empty()) || glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::SetAttachment(const TextureHandle& handle, uint32_t index){
		OpenGLTexture2D* texture = ((OpenGLTexture2D*)handle.Texture.Get());
		KD_CORE_ASSERT(index < m_ColorAttachmentSpecifications.size() && texture->m_TextureFormat == m_ColorAttachmentSpecifications.at(index).Format);
		glNamedFramebufferTexture(m_RendererID, GL_COLOR_ATTACHMENT0 + index,texture->m_RendererID, 0);
	}
	void OpenGLFramebuffer::SetAttachment(const TextureArrayHandle& handle, uint32_t index){
		OpenGLTexture2DArray* array = ((OpenGLTexture2DArray*)handle.Array.Get());
		KD_CORE_ASSERT(index < m_ColorAttachmentSpecifications.size() && array->m_TextureFormat == m_ColorAttachmentSpecifications.at(index).Format);
		glNamedFramebufferTextureLayer(m_RendererID, GL_COLOR_ATTACHMENT0 + index,array->m_RendererID, 0, (GLint)handle.SlotIndex);
	}
	void OpenGLFramebuffer::SetDepthAttachment(const TextureHandle& handle){
		OpenGLTexture2D* texture = ((OpenGLTexture2D*)handle.Texture.Get());
		KD_CORE_ASSERT(m_DepthAttachment&& texture->m_TextureFormat == m_DepthAttachmentSpecification.Format);
		glNamedFramebufferTexture(m_RendererID, GL_DEPTH_ATTACHMENT, texture->m_RendererID, 0);
	}
	void OpenGLFramebuffer::SetDepthAttachment(const TextureArrayHandle& handle) {
		OpenGLTexture2DArray* array = ((OpenGLTexture2DArray*)handle.Array.Get());
		KD_CORE_ASSERT(m_DepthAttachment && array->m_TextureFormat == m_DepthAttachmentSpecification.Format);
		glNamedFramebufferTextureLayer(m_RendererID,GL_DEPTH_ATTACHMENT , array->m_RendererID, 0, (GLint)handle.SlotIndex);
	}
}
