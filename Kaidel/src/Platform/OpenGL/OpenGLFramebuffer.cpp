#include "KDpch.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

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
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;
			BindTexture(multisampled, id);
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_TRUE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D,0,format,width,height,GL_FALSE,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8:  return true;
				case FramebufferTextureFormat::DEPTH32: return true;
				case FramebufferTextureFormat::DEPTH16: return true;
			}

			return false;
		}

		static GLenum KaidelFBTextureFormatToGL(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case FramebufferTextureFormat::RGBA8:       return GL_RGBA8;
				case FramebufferTextureFormat::R32I: return GL_R32I;
				case FramebufferTextureFormat::RGBA32F: return GL_RGBA32F;

			}

			KD_CORE_ASSERT(false);
			return 0;
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
		}


		static GLenum KaidelFBTextureFormatToGLInternalFormat(FramebufferTextureFormat format) {
			switch (format)
			{
			case Kaidel::FramebufferTextureFormat::RGBA32F			: return GL_RGBA32F;
			case Kaidel::FramebufferTextureFormat::RGBA32UI			: return GL_RGBA32UI;
			case Kaidel::FramebufferTextureFormat::RGBA32I			: return GL_RGBA32I;
			case Kaidel::FramebufferTextureFormat::RGB32F			: return GL_RGB32F;
			case Kaidel::FramebufferTextureFormat::RGB32UI			: return GL_RGB32UI;
			case Kaidel::FramebufferTextureFormat::RGB32I			: return GL_RGB32I;
			case Kaidel::FramebufferTextureFormat::RGBA16F			: return GL_RGBA16F;
			case Kaidel::FramebufferTextureFormat::RGBA16			: return GL_RGBA16;
			case Kaidel::FramebufferTextureFormat::RGBA16UI			: return GL_RGBA16UI;
			case Kaidel::FramebufferTextureFormat::RGBA16NORM		: return GL_RGBA16_SNORM;
			case Kaidel::FramebufferTextureFormat::RGBA16I			: return GL_RGBA16I;
			case Kaidel::FramebufferTextureFormat::RG32F			: return GL_RG32F;
			case Kaidel::FramebufferTextureFormat::RG32UI			: return GL_RG32UI;
			case Kaidel::FramebufferTextureFormat::RG32I			: return GL_RG32I;
			case Kaidel::FramebufferTextureFormat::RGBA8			: return GL_RGBA8;
			case Kaidel::FramebufferTextureFormat::RGBA8UI			: return GL_RGBA8UI;
			case Kaidel::FramebufferTextureFormat::RGBA8NORM		: return GL_RGBA8_SNORM;
			case Kaidel::FramebufferTextureFormat::RGBA8I			: return GL_RGBA8I;
			case Kaidel::FramebufferTextureFormat::RG16F			: return GL_RG16F;
			case Kaidel::FramebufferTextureFormat::RG16				: return GL_RG16;
			case Kaidel::FramebufferTextureFormat::RG16UI			: return GL_RG16UI;
			case Kaidel::FramebufferTextureFormat::RG16NORM			: return GL_RG16_SNORM;
			case Kaidel::FramebufferTextureFormat::RG16I			: return GL_RG16I;
			case Kaidel::FramebufferTextureFormat::R32F				: return GL_R32F;
			case Kaidel::FramebufferTextureFormat::R32UI			: return GL_R32UI;
			case Kaidel::FramebufferTextureFormat::R32I				: return GL_R32I;
			case Kaidel::FramebufferTextureFormat::RG8				: return GL_RG8;
			case Kaidel::FramebufferTextureFormat::RG8UI			: return GL_RG8UI;
			case Kaidel::FramebufferTextureFormat::RG8NORM			: return GL_RG8_SNORM;
			case Kaidel::FramebufferTextureFormat::RG8I				: return GL_RG8I;
			case Kaidel::FramebufferTextureFormat::R16F				: return GL_R16F;
			case Kaidel::FramebufferTextureFormat::R16				: return GL_R16;
			case Kaidel::FramebufferTextureFormat::R16UI			: return GL_R16UI;
			case Kaidel::FramebufferTextureFormat::R16NORM			: return GL_R16_SNORM;
			case Kaidel::FramebufferTextureFormat::R16I				: return GL_R16I;
			case Kaidel::FramebufferTextureFormat::R8				: return GL_R8;
			case Kaidel::FramebufferTextureFormat::R8UI				: return GL_R8UI;
			case Kaidel::FramebufferTextureFormat::R8NORM			: return GL_R8_SNORM;
			case Kaidel::FramebufferTextureFormat::R8I				: return GL_R8I;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return GL_NONE;
		}
		static GLenum KaidelFBTextureFormatToGLFormat(FramebufferTextureFormat format) {
			switch (format)
			{
			case Kaidel::FramebufferTextureFormat::RGBA32F: return GL_RGBA;
			case Kaidel::FramebufferTextureFormat::RGBA32UI:return GL_RGBA_INTEGER;
			case Kaidel::FramebufferTextureFormat::RGBA32I: return GL_RGBA_INTEGER;
			case Kaidel::FramebufferTextureFormat::RGB32F:return GL_RGB;
			case Kaidel::FramebufferTextureFormat::RGB32UI:return GL_RGB_INTEGER;
			case Kaidel::FramebufferTextureFormat::RGB32I: return GL_RGB_INTEGER;
			case Kaidel::FramebufferTextureFormat::RGBA16F:return GL_RGBA;
			case Kaidel::FramebufferTextureFormat::RGBA16:return GL_RGBA;
			case Kaidel::FramebufferTextureFormat::RGBA16UI:return GL_RGBA_INTEGER;
			case Kaidel::FramebufferTextureFormat::RGBA16NORM:return GL_RGBA;
			case Kaidel::FramebufferTextureFormat::RGBA16I: return GL_RGBA;
			case Kaidel::FramebufferTextureFormat::RG32F: return GL_RG;
			case Kaidel::FramebufferTextureFormat::RG32UI:return GL_RG_INTEGER;
			case Kaidel::FramebufferTextureFormat::RG32I:return GL_RG_INTEGER;
			case Kaidel::FramebufferTextureFormat::RGBA8: return GL_RGBA;
			case Kaidel::FramebufferTextureFormat::RGBA8UI: return GL_RGBA_INTEGER;
			case Kaidel::FramebufferTextureFormat::RGBA8NORM: return GL_RGBA;
			case Kaidel::FramebufferTextureFormat::RGBA8I: return GL_RGBA_INTEGER;
			case Kaidel::FramebufferTextureFormat::RG16F: return GL_RG;
			case Kaidel::FramebufferTextureFormat::RG16: return GL_RG;
			case Kaidel::FramebufferTextureFormat::RG16UI: return GL_RG_INTEGER;
			case Kaidel::FramebufferTextureFormat::RG16NORM: return GL_RG;
			case Kaidel::FramebufferTextureFormat::RG16I: return GL_RG_INTEGER;
			case Kaidel::FramebufferTextureFormat::R32F: return GL_RED;
			case Kaidel::FramebufferTextureFormat::R32UI: return GL_RED_INTEGER;
			case Kaidel::FramebufferTextureFormat::R32I: return GL_RED_INTEGER;
			case Kaidel::FramebufferTextureFormat::RG8: return GL_RG;
			case Kaidel::FramebufferTextureFormat::RG8UI: return GL_RG_INTEGER;
			case Kaidel::FramebufferTextureFormat::RG8NORM: return GL_RG;
			case Kaidel::FramebufferTextureFormat::RG8I: return GL_RG_INTEGER;
			case Kaidel::FramebufferTextureFormat::R16F: return GL_RED;
			case Kaidel::FramebufferTextureFormat::R16: return GL_RED;
			case Kaidel::FramebufferTextureFormat::R16UI: return GL_RED_INTEGER;
			case Kaidel::FramebufferTextureFormat::R16NORM: return GL_RED;
			case Kaidel::FramebufferTextureFormat::R16I: return GL_RED_INTEGER;
			case Kaidel::FramebufferTextureFormat::R8: return GL_RED;
			case Kaidel::FramebufferTextureFormat::R8UI: return GL_RED_INTEGER;
			case Kaidel::FramebufferTextureFormat::R8NORM: return GL_RED;
			case Kaidel::FramebufferTextureFormat::R8I: return GL_RED_INTEGER;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return GL_NONE;
		}



	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto spec : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
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
		glBindImageTexture(slot,m_ColorAttachments[attachmnetIndex], 0, GL_FALSE, 0, Utils::KaidelImageBindingToGL(bindingMode), Utils::KaidelFBTextureFormatToGLInternalFormat(m_ColorAttachmentSpecifications[attachmnetIndex].TextureFormat));
	}


	void OpenGLFramebuffer::CopyColorAttachment(uint32_t dstAttachmentIndex, uint32_t srcAttachmentIndex, Ref<Framebuffer> src) {
		auto oglSrc = std::dynamic_pointer_cast<OpenGLFramebuffer>(src);
		if (dstAttachmentIndex >= m_Specification.Attachments.Attachments.size() || srcAttachmentIndex >= oglSrc->m_Specification.Attachments.Attachments.size())
			return;
		if (oglSrc->m_Specification.Width != m_Specification.Width || oglSrc->m_Specification.Height != m_Specification.Height)
			return;
		bool multiSampled = m_Specification.Samples > 1;
		bool srcMultiSampled = oglSrc->m_Specification.Samples > 1;
		glCopyImageSubData(oglSrc->m_ColorAttachments[srcAttachmentIndex], Utils::TextureTarget(srcMultiSampled), 0, 0, 0, 0, m_ColorAttachments[dstAttachmentIndex], Utils::TextureTarget(multiSampled), 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, 1);
	}
	void OpenGLFramebuffer::CopyDepthAttachment(Ref<Framebuffer> src) {
		auto oglSrc = std::dynamic_pointer_cast<OpenGLFramebuffer>(src);
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
				glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			if (m_DepthAttachment)
				glDeleteTextures(1, &m_DepthAttachment);

			if (glfwGetCurrentContext() != NULL) {
				std::cout << "Here\n";
			}

			glDeleteFramebuffers(1, &m_RendererID);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}
	}


	void OpenGLFramebuffer::SetDepthAttachmentFromArray(uint32_t attachmentID, uint32_t arrayIndex) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, attachmentID, 0, arrayIndex);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::ClearDepthAttachment(float value) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glClear(GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			Unbind();
			if (m_ColorAttachments.size())
				glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			if (m_DepthAttachment)
				glDeleteTextures(1, &m_DepthAttachment);

			glDeleteFramebuffers(1, &m_RendererID);
			
			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
			m_RendererID = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);


		if (m_Specification.Attachments.Attachments.empty())
		{
			glBindFramebuffer(GL_FRAMEBUFFER,m_RendererID);
			glDrawBuffer(GL_NONE);
		}

		bool multisample = m_Specification.Samples > 1;

		// Attachments
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multisample, m_ColorAttachments[i]);
				Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, Utils::KaidelFBTextureFormatToGLInternalFormat(m_ColorAttachmentSpecifications[i].TextureFormat), 
							Utils::KaidelFBTextureFormatToGLFormat(m_ColorAttachmentSpecifications[i].TextureFormat), m_Specification.Width, m_Specification.Height, i);
			}
		}

		if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, m_DepthAttachment);
			switch (m_DepthAttachmentSpecification.TextureFormat)
			{
				case FramebufferTextureFormat::DEPTH32:
					Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
					break;
				case FramebufferTextureFormat::DEPTH16:
					Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT16, GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
					break;
			}
		}

		if (m_ColorAttachments.size() >= 1)
		{
			KD_CORE_ASSERT(m_ColorAttachments.size() <= 4);
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else {
			glDrawBuffer(GL_NONE);
		}
		
		if (m_Specification.Attachments.Attachments.empty())
			return;
		KD_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
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

	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		KD_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;

	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		KD_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0,
			Utils::KaidelFBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, const float* colors)
	{
		KD_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		auto& i = m_ColorAttachmentSpecifications.at(attachmentIndex);
		if (i.TextureFormat == FramebufferTextureFormat::RGBA8) {
			glClearColor(colors[0], colors[1], colors[2], colors[3]);
			glClear(GL_COLOR_BUFFER_BIT);
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}
