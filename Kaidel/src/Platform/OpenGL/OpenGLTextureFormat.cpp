#include "KDpch.h"
#include "OpenGLTextureFormat.h"
namespace Kaidel {
	namespace Utils {
		GLenum KaidelTextureFormatToGLInternalFormat(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F:		return GL_RGBA32F;
			case Kaidel::TextureFormat::RGBA32UI:		return GL_RGBA32UI;
			case Kaidel::TextureFormat::RGBA32I:		return GL_RGBA32I;
			case Kaidel::TextureFormat::RGB32F:		return GL_RGB32F;
			case Kaidel::TextureFormat::RGB32UI:		return GL_RGB32UI;
			case Kaidel::TextureFormat::RGB32I:		return GL_RGB32I;
			case Kaidel::TextureFormat::RGBA16F:		return GL_RGBA16F;
			case Kaidel::TextureFormat::RGBA16:		return GL_RGBA16;
			case Kaidel::TextureFormat::RGBA16UI:		return GL_RGBA16UI;
			case Kaidel::TextureFormat::RGBA16NORM:		return GL_RGBA16_SNORM;
			case Kaidel::TextureFormat::RGBA16I:		return GL_RGBA16I;
			case Kaidel::TextureFormat::RG32F:		return GL_RG32F;
			case Kaidel::TextureFormat::RG32UI:		return GL_RG32UI;
			case Kaidel::TextureFormat::RG32I:		return GL_RG32I;
			case Kaidel::TextureFormat::RGBA8:		return GL_RGBA8;
			case Kaidel::TextureFormat::RGBA8UI:		return GL_RGBA8UI;
			case Kaidel::TextureFormat::RGBA8NORM:		return GL_RGBA8_SNORM;
			case Kaidel::TextureFormat::RGBA8I:		return GL_RGBA8I;
			case Kaidel::TextureFormat::RG16F:		return GL_RG16F;
			case Kaidel::TextureFormat::RG16:		return GL_RG16;
			case Kaidel::TextureFormat::RG16UI:		return GL_RG16UI;
			case Kaidel::TextureFormat::RG16NORM:		return GL_RG16_SNORM;
			case Kaidel::TextureFormat::RG16I:		return GL_RG16I;
			case Kaidel::TextureFormat::R32F:		return GL_R32F;
			case Kaidel::TextureFormat::R32UI:		return GL_R32UI;
			case Kaidel::TextureFormat::R32I:		return GL_R32I;
			case Kaidel::TextureFormat::RG8:		return GL_RG8;
			case Kaidel::TextureFormat::RG8UI:		return GL_RG8UI;
			case Kaidel::TextureFormat::RG8NORM:		return GL_RG8_SNORM;
			case Kaidel::TextureFormat::RG8I:		return GL_RG8I;
			case Kaidel::TextureFormat::R16F:		return GL_R16F;
			case Kaidel::TextureFormat::R16:		return GL_R16;
			case Kaidel::TextureFormat::R16UI:		return GL_R16UI;
			case Kaidel::TextureFormat::R16NORM:		return GL_R16_SNORM;
			case Kaidel::TextureFormat::R16I:		return GL_R16I;
			case Kaidel::TextureFormat::R8:		return GL_R8;
			case Kaidel::TextureFormat::R8UI:		return GL_R8UI;
			case Kaidel::TextureFormat::R8NORM:		return GL_R8_SNORM;
			case Kaidel::TextureFormat::R8I:		return GL_R8I;
			case Kaidel::TextureFormat::Depth16F:		return GL_DEPTH_COMPONENT16;
			case Kaidel::TextureFormat::Depth32F:		return GL_DEPTH_COMPONENT32F;
			case Kaidel::TextureFormat::Depth32:		return GL_DEPTH_COMPONENT32;
			case Kaidel::TextureFormat::Depth24Stencil8:		return GL_DEPTH24_STENCIL8;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return GL_NONE;
		}
		GLenum KaidelTextureFormatToGLFormat(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA32UI:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGBA32I:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGB32F:		return GL_RGB;
			case Kaidel::TextureFormat::RGB32UI:		return GL_RGB_INTEGER;
			case Kaidel::TextureFormat::RGB32I:		return GL_RGB_INTEGER;
			case Kaidel::TextureFormat::RGBA16F:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA16:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA16UI:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGBA16NORM:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA16I:		return GL_RGBA;
			case Kaidel::TextureFormat::RG32F:		return GL_RG;
			case Kaidel::TextureFormat::RG32UI:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RG32I:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RGBA8:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA8UI:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RGBA8NORM:		return GL_RGBA;
			case Kaidel::TextureFormat::RGBA8I:		return GL_RGBA_INTEGER;
			case Kaidel::TextureFormat::RG16F:		return GL_RG;
			case Kaidel::TextureFormat::RG16:		return GL_RG;
			case Kaidel::TextureFormat::RG16UI:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RG16NORM:		return GL_RG;
			case Kaidel::TextureFormat::RG16I:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::R32F:		return GL_RED;
			case Kaidel::TextureFormat::R32UI:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R32I:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::RG8:		return GL_RG;
			case Kaidel::TextureFormat::RG8UI:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::RG8NORM:		return GL_RG;
			case Kaidel::TextureFormat::RG8I:		return GL_RG_INTEGER;
			case Kaidel::TextureFormat::R16F:		return GL_RED;
			case Kaidel::TextureFormat::R16:		return GL_RED;
			case Kaidel::TextureFormat::R16UI:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R16NORM:		return GL_RED;
			case Kaidel::TextureFormat::R16I:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R8:		return GL_RED;
			case Kaidel::TextureFormat::R8UI:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::R8NORM:		return GL_RED;
			case Kaidel::TextureFormat::R8I:		return GL_RED_INTEGER;
			case Kaidel::TextureFormat::Depth16F:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth32F:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth32:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth24Stencil8:		return GL_DEPTH_STENCIL;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return GL_NONE;
		}

		GLenum KaidelTextureFormatToGLValueFormat(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F:		return GL_FLOAT;
			case Kaidel::TextureFormat::RGBA32UI:		return GL_UNSIGNED_INT;
			case Kaidel::TextureFormat::RGBA32I:		return GL_INT;
			case Kaidel::TextureFormat::RGB32F:		return GL_FLOAT;
			case Kaidel::TextureFormat::RGB32UI:		return GL_UNSIGNED_INT;
			case Kaidel::TextureFormat::RGB32I:		return GL_INT;
			case Kaidel::TextureFormat::RGBA16F:		return GL_HALF_FLOAT;
			case Kaidel::TextureFormat::RGBA16:		return GL_UNSIGNED_SHORT;
			case Kaidel::TextureFormat::RGBA16UI:		return GL_UNSIGNED_INT;
			case Kaidel::TextureFormat::RGBA16NORM:		return GL_SHORT;
			case Kaidel::TextureFormat::RGBA16I:		return GL_INT;
			case Kaidel::TextureFormat::RG32F:		return GL_FLOAT;
			case Kaidel::TextureFormat::RG32UI:		return GL_UNSIGNED_INT;
			case Kaidel::TextureFormat::RG32I:		return GL_INT;
			case Kaidel::TextureFormat::RGBA8:		return GL_UNSIGNED_BYTE;
			case Kaidel::TextureFormat::RGBA8UI:		return GL_UNSIGNED_BYTE;
			case Kaidel::TextureFormat::RGBA8NORM:		return GL_BYTE;
			case Kaidel::TextureFormat::RGBA8I:		return GL_BYTE;
			case Kaidel::TextureFormat::RG16F:		return GL_HALF_FLOAT;
			case Kaidel::TextureFormat::RG16:		return GL_SHORT;
			case Kaidel::TextureFormat::RG16UI:		return GL_UNSIGNED_INT;
			case Kaidel::TextureFormat::RG16NORM:		return GL_SHORT;
			case Kaidel::TextureFormat::RG16I:		return GL_SHORT;
			case Kaidel::TextureFormat::R32F:		return GL_FLOAT;
			case Kaidel::TextureFormat::R32UI:		return GL_UNSIGNED_INT;
			case Kaidel::TextureFormat::R32I:		return GL_INT;
			case Kaidel::TextureFormat::RG8:		return GL_UNSIGNED_BYTE;
			case Kaidel::TextureFormat::RG8UI:		return GL_UNSIGNED_BYTE;
			case Kaidel::TextureFormat::RG8NORM:		return GL_BYTE;
			case Kaidel::TextureFormat::RG8I:		return GL_BYTE;
			case Kaidel::TextureFormat::R16F:		return GL_HALF_FLOAT;
			case Kaidel::TextureFormat::R16:		return GL_UNSIGNED_SHORT;
			case Kaidel::TextureFormat::R16UI:		return GL_UNSIGNED_SHORT;
			case Kaidel::TextureFormat::R16NORM:		return GL_SHORT;
			case Kaidel::TextureFormat::R16I:		return GL_SHORT;
			case Kaidel::TextureFormat::R8:		return GL_UNSIGNED_BYTE;
			case Kaidel::TextureFormat::R8UI:		return GL_UNSIGNED_BYTE;
			case Kaidel::TextureFormat::R8NORM:		return GL_BYTE;
			case Kaidel::TextureFormat::R8I:		return GL_BYTE;
			case Kaidel::TextureFormat::Depth16F:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth32F:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth32:		return GL_DEPTH_COMPONENT;
			case Kaidel::TextureFormat::Depth24Stencil8:		return GL_DEPTH_STENCIL;

			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return GL_NONE;
		}

	}
}
