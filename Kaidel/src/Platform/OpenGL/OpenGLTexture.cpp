#include "KDpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Platform/OpenGL/OpenGLTextureFormat.h"
#include "Kaidel/Core/Timer.h"
#include <stb_image.h>
#include <stb_image_resize.h>

namespace Kaidel {


	


	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format)
		: m_Width(width), m_Height(height),m_TextureFormat(format)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, Utils::KaidelTextureFormatToGLInternalFormat(m_TextureFormat), m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(path.c_str(), &width, &height, &channels,4);
		}
		KD_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			m_TextureFormat = TextureFormat::RGBA8;
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}


		KD_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, Utils::KaidelTextureFormatToGLInternalFormat(m_TextureFormat), m_Width, m_Height, GL_FALSE, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_UNSIGNED_BYTE, data);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Reset(void* data, uint32_t width, uint32_t height) {
		if (m_Width != width || m_Height != height) {
			glDeleteTextures(1, &m_RendererID);
			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glBindTexture(GL_TEXTURE_2D, m_RendererID);
			glTexImage2D(GL_TEXTURE_2D, 0, Utils::KaidelTextureFormatToGLInternalFormat(m_TextureFormat), width, height, GL_FALSE, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_UNSIGNED_BYTE, data);
			m_Width = width;
			m_Height = height;
			return;
		}
		SetData(data, 0);
	}
	void OpenGLTexture2D::Bind(uint32_t slot) const
	{

		glBindTextureUnit(slot, m_RendererID);
	}


	const std::string& OpenGLTexture2D::GetPath() const
	{
		return m_Path;
	}




	TextureHandle OpenGLTexture2D::GetHandle()const {
		TextureHandle handle{};
		handle.Texture = const_cast<OpenGLTexture2D*>(this);
		return handle;
	}
	
}
