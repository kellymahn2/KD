#include "KDpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Kaidel/Core/Timer.h"
#include <stb_image.h>
#include <stb_image_resize.h>

namespace Kaidel {


	namespace Utils {
		static GLenum KaidelFBTextureFormatToGLInternalFormat(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F				:		return GL_RGBA32F;
			case Kaidel::TextureFormat::RGBA32UI			:		return GL_RGBA32UI;
			case Kaidel::TextureFormat::RGBA32I				:		return GL_RGBA32I;
			case Kaidel::TextureFormat::RGB32F				:		return GL_RGB32F;
			case Kaidel::TextureFormat::RGB32UI				:		return GL_RGB32UI;
			case Kaidel::TextureFormat::RGB32I				:		return GL_RGB32I;
			case Kaidel::TextureFormat::RGBA16F				:		return GL_RGBA16F;
			case Kaidel::TextureFormat::RGBA16				:		return GL_RGBA16;
			case Kaidel::TextureFormat::RGBA16UI			:		return GL_RGBA16UI;
			case Kaidel::TextureFormat::RGBA16NORM			:		return GL_RGBA16_SNORM;
			case Kaidel::TextureFormat::RGBA16I				:		return GL_RGBA16I;
			case Kaidel::TextureFormat::RG32F				:		return GL_RG32F;
			case Kaidel::TextureFormat::RG32UI				:		return GL_RG32UI;
			case Kaidel::TextureFormat::RG32I				:		return GL_RG32I;
			case Kaidel::TextureFormat::RGBA8				:		return GL_RGBA8;
			case Kaidel::TextureFormat::RGBA8UI				:		return GL_RGBA8UI;
			case Kaidel::TextureFormat::RGBA8NORM			:		return GL_RGBA8_SNORM;
			case Kaidel::TextureFormat::RGBA8I				:		return GL_RGBA8I;
			case Kaidel::TextureFormat::RG16F				:		return GL_RG16F;
			case Kaidel::TextureFormat::RG16				:		return GL_RG16;
			case Kaidel::TextureFormat::RG16UI				:		return GL_RG16UI;
			case Kaidel::TextureFormat::RG16NORM			:		return GL_RG16_SNORM;
			case Kaidel::TextureFormat::RG16I				:		return GL_RG16I;
			case Kaidel::TextureFormat::R32F				:		return GL_R32F;
			case Kaidel::TextureFormat::R32UI				:		return GL_R32UI;
			case Kaidel::TextureFormat::R32I				:		return GL_R32I;
			case Kaidel::TextureFormat::RG8					:		return GL_RG8;
			case Kaidel::TextureFormat::RG8UI				:		return GL_RG8UI;
			case Kaidel::TextureFormat::RG8NORM				:		return GL_RG8_SNORM;
			case Kaidel::TextureFormat::RG8I				:		return GL_RG8I;
			case Kaidel::TextureFormat::R16F				:		return GL_R16F;
			case Kaidel::TextureFormat::R16					:		return GL_R16;
			case Kaidel::TextureFormat::R16UI				:		return GL_R16UI;
			case Kaidel::TextureFormat::R16NORM				:		return GL_R16_SNORM;
			case Kaidel::TextureFormat::R16I				:		return GL_R16I;
			case Kaidel::TextureFormat::R8					:		return GL_R8;
			case Kaidel::TextureFormat::R8UI				:		return GL_R8UI;
			case Kaidel::TextureFormat::R8NORM				:		return GL_R8_SNORM;
			case Kaidel::TextureFormat::R8I					:		return GL_R8I;
			case Kaidel::TextureFormat::Depth16F			:		return GL_DEPTH_COMPONENT16;
			case Kaidel::TextureFormat::Depth32F			:		return GL_DEPTH_COMPONENT32F;
			case Kaidel::TextureFormat::Depth32				:		return GL_DEPTH_COMPONENT32;
			case Kaidel::TextureFormat::Depth24Stencil8		:		return GL_DEPTH24_STENCIL8;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return GL_NONE;
		}
		static GLenum KaidelFBTextureFormatToGLFormat(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F				:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA32UI			:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGBA32I				:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGB32F				:		return GL_RGB;
			case Kaidel::TextureFormat::RGB32UI				:		return GL_RGB_INTEGER;
			case Kaidel::TextureFormat::RGB32I				:		return GL_RGB_INTEGER;
			case Kaidel::TextureFormat::RGBA16F				:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA16				:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA16UI			:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGBA16NORM			:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA16I				:		return GL_RGBA;
			case Kaidel::TextureFormat::RG32F				:		return GL_RG;
			case Kaidel::TextureFormat::RG32UI				:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RG32I				:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RGBA8				:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA8UI				:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGBA8NORM			:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA8I				:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RG16F				:		return GL_RG;
			case Kaidel::TextureFormat::RG16				:		return GL_RG;
			case Kaidel::TextureFormat::RG16UI				:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RG16NORM			:		return GL_RG;
			case Kaidel::TextureFormat::RG16I				:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::R32F				:		return GL_RED;
			case Kaidel::TextureFormat::R32UI				:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R32I				:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::RG8					:		return GL_RG;
			case Kaidel::TextureFormat::RG8UI				:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RG8NORM				:		return GL_RG;
			case Kaidel::TextureFormat::RG8I				:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::R16F				:		return GL_RED;
			case Kaidel::TextureFormat::R16					:		return GL_RED;
			case Kaidel::TextureFormat::R16UI				:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R16NORM				:		return GL_RED;
			case Kaidel::TextureFormat::R16I				:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R8					:		return GL_RED;
			case Kaidel::TextureFormat::R8UI				:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R8NORM				:		return GL_RED;
			case Kaidel::TextureFormat::R8I					:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::Depth16F			:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth32F			:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth32				:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth24Stencil8		:		return GL_DEPTH_STENCIL;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return GL_NONE;
		}
	}


	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format)
		: m_Width(width), m_Height(height),m_TextureFormat(format)
	{




		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, Utils::KaidelFBTextureFormatToGLInternalFormat(m_TextureFormat), m_Width, m_Height);

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
		glTexImage2D(GL_TEXTURE_2D, 0, Utils::KaidelFBTextureFormatToGLInternalFormat(m_TextureFormat), m_Width, m_Height, GL_FALSE, Utils::KaidelFBTextureFormatToGLFormat(m_TextureFormat), GL_UNSIGNED_BYTE, data);

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
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, Utils::KaidelFBTextureFormatToGLFormat(m_TextureFormat), GL_UNSIGNED_BYTE, data);
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
