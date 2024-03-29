#include "KDpch.h"
#include "OpenGLTextureFormat.h"
#include "Kaidel/Core/ImageResize.h"
#include <stb_image_resize.h>
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

		uint32_t KaidelTextureFormatComponentCount(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F:		return 4;
			case Kaidel::TextureFormat::RGBA32UI:		return 4;
			case Kaidel::TextureFormat::RGBA32I:		return 4;
			case Kaidel::TextureFormat::RGB32F:		return 3;
			case Kaidel::TextureFormat::RGB32UI:		return 3;
			case Kaidel::TextureFormat::RGB32I:		return 3;
			case Kaidel::TextureFormat::RGBA16F:		return 4;
			case Kaidel::TextureFormat::RGBA16UI:		return 4;
			case Kaidel::TextureFormat::RGBA16NORM:		return 4;
			case Kaidel::TextureFormat::RGBA16I:		return 4;
			case Kaidel::TextureFormat::RG32F:		return 2;
			case Kaidel::TextureFormat::RG32UI:		return 2;
			case Kaidel::TextureFormat::RG32I:		return 2;
			case Kaidel::TextureFormat::RGBA8:		return 4;
			case Kaidel::TextureFormat::RGBA8UI:		return 4;
			case Kaidel::TextureFormat::RGBA8NORM:		return 4;
			case Kaidel::TextureFormat::RGBA8I:		return 4;
			case Kaidel::TextureFormat::RG16F:		return 2;
			case Kaidel::TextureFormat::RG16UI:		return 2;
			case Kaidel::TextureFormat::RG16NORM:		return 2;
			case Kaidel::TextureFormat::RG16I:		return 2;
			case Kaidel::TextureFormat::R32F:		return 1;
			case Kaidel::TextureFormat::R32UI:		return 1;
			case Kaidel::TextureFormat::R32I:		return 1;
			case Kaidel::TextureFormat::RG8:		return 2;
			case Kaidel::TextureFormat::RG8UI:		return 2;
			case Kaidel::TextureFormat::RG8NORM:		return 2;
			case Kaidel::TextureFormat::RG8I:		return 2;
			case Kaidel::TextureFormat::R16F:		return 1;
			case Kaidel::TextureFormat::R16UI:		return 1;
			case Kaidel::TextureFormat::R16NORM:		return 1;
			case Kaidel::TextureFormat::R16I:		return 1;
			case Kaidel::TextureFormat::R8:		return 1;
			case Kaidel::TextureFormat::R8UI:		return 1;
			case Kaidel::TextureFormat::R8NORM:		return 1;
			case Kaidel::TextureFormat::R8I:		return 1;
			case Kaidel::TextureFormat::Depth16F:		return 1;
			case Kaidel::TextureFormat::Depth32F:		return 1;
			case Kaidel::TextureFormat::Depth32:		return 1;
			case Kaidel::TextureFormat::Depth24Stencil8:		return 2;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return 0;
		}

		uint32_t KaidelTextureFormatChannelSizeInBytes(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F:		return 4;
			case Kaidel::TextureFormat::RGBA32UI:		return 4;
			case Kaidel::TextureFormat::RGBA32I:		return 4;
			case Kaidel::TextureFormat::RGB32F:		return 3;
			case Kaidel::TextureFormat::RGB32UI:		return 3;
			case Kaidel::TextureFormat::RGB32I:		return 3;
			case Kaidel::TextureFormat::RGBA16F:		return 2;
			case Kaidel::TextureFormat::RGBA16UI:		return 2;
			case Kaidel::TextureFormat::RGBA16NORM:		return 2;
			case Kaidel::TextureFormat::RGBA16I:		return 2;
			case Kaidel::TextureFormat::RG32F:		return 4;
			case Kaidel::TextureFormat::RG32UI:		return 4;
			case Kaidel::TextureFormat::RG32I:		return 4;
			case Kaidel::TextureFormat::RGBA8:		return 1;
			case Kaidel::TextureFormat::RGBA8UI:		return 1;
			case Kaidel::TextureFormat::RGBA8NORM:		return 1;
			case Kaidel::TextureFormat::RGBA8I:		return 1;
			case Kaidel::TextureFormat::RG16F:		return 2;
			case Kaidel::TextureFormat::RG16UI:		return 2;
			case Kaidel::TextureFormat::RG16NORM:		return 2;
			case Kaidel::TextureFormat::RG16I:		return 2;
			case Kaidel::TextureFormat::R32F:		return 4;
			case Kaidel::TextureFormat::R32UI:		return 4;
			case Kaidel::TextureFormat::R32I:		return 4;
			case Kaidel::TextureFormat::RG8:		return 1;
			case Kaidel::TextureFormat::RG8UI:		return 1;
			case Kaidel::TextureFormat::RG8NORM:		return 1;
			case Kaidel::TextureFormat::RG8I:		return 1;
			case Kaidel::TextureFormat::R16F:		return 2;
			case Kaidel::TextureFormat::R16UI:		return 2;
			case Kaidel::TextureFormat::R16NORM:		return 2;
			case Kaidel::TextureFormat::R16I:		return 2;
			case Kaidel::TextureFormat::R8:		return 1;
			case Kaidel::TextureFormat::R8UI:		return 1;
			case Kaidel::TextureFormat::R8NORM:		return 1;
			case Kaidel::TextureFormat::R8I:		return 1;
			case Kaidel::TextureFormat::Depth16F:		return 2;
			case Kaidel::TextureFormat::Depth32F:		return 4;
			case Kaidel::TextureFormat::Depth32:		return 4;
			case Kaidel::TextureFormat::Depth24Stencil8:		return 4;
			}
			KD_ERROR("{} Is not a valid framebuffer texture type");
			return 0;
		}
		uint32_t KaidelTextureFormatPixelSizeInBytes(TextureFormat format) {
			return KaidelTextureFormatComponentCount(format) * KaidelTextureFormatChannelSizeInBytes(format);
		}


		void* ResizeTexture(void* data, TextureFormat iFormat, uint32_t iW, uint32_t iH, uint32_t oW, uint32_t oH) {
			int channelCount = KaidelTextureFormatComponentCount(iFormat);
			int pixelSize = KaidelTextureFormatPixelSizeInBytes(iFormat);
#define RESIZE(channel_type,org_image,org_width,org_height,func,new_width,new_height,num_channels) {channel_type* out = new channel_type[new_width * new_height * num_channels];\
			KD_CORE_ASSERT(func((const channel_type*)org_image,org_width,org_height,out,new_width,new_height,num_channels) == RESIZE_COMPLETE);return out;}
			switch (iFormat)
			{
			case Kaidel::TextureFormat::RGBA32F:	RESIZE(float, data, iW, iH, resize_float, oW, oH, 4);
			case Kaidel::TextureFormat::RGBA32UI:	RESIZE(uint32_t, data, iW, iH, resize_uint32, oW, oH, 4);
			case Kaidel::TextureFormat::RGBA32I:	RESIZE(int32_t, data, iW, iH, resize_int32, oW, oH, 4);
			case Kaidel::TextureFormat::RGB32F:	RESIZE(float, data, iW, iH, resize_float, oW, oH, 3);
			case Kaidel::TextureFormat::RGB32UI:	RESIZE(uint32_t, data, iW, iH, resize_uint32, oW, oH, 3);
			case Kaidel::TextureFormat::RGB32I:	RESIZE(int32_t, data, iW, iH, resize_int32, oW, oH, 3);
			case Kaidel::TextureFormat::RGBA16F:	return nullptr;
			case Kaidel::TextureFormat::RGBA16UI:	return nullptr;
			case Kaidel::TextureFormat::RGBA16NORM:	return nullptr;
			case Kaidel::TextureFormat::RGBA16I:	return nullptr;
			case Kaidel::TextureFormat::RG32F:	RESIZE(float, data, iW, iH, resize_float, oW, oH, 2);
			case Kaidel::TextureFormat::RG32UI:	RESIZE(uint32_t, data, iW, iH, resize_uint32, oW, oH, 2);
			case Kaidel::TextureFormat::RG32I:	RESIZE(int32_t, data, iW, iH, resize_int32, oW, oH, 2);
			case Kaidel::TextureFormat::RGBA8:	RESIZE(uint8_t, data, iW, iH, resize_uint8_normalized, oW, oH, 4);
			case Kaidel::TextureFormat::RGBA8UI:	return nullptr;
			case Kaidel::TextureFormat::RGBA8NORM:	RESIZE(int8_t, data, iW, iH, resize_int8_normalized, oW, oH, 4);
			case Kaidel::TextureFormat::RGBA8I:	return nullptr;
			case Kaidel::TextureFormat::RG16F:	return nullptr;
			case Kaidel::TextureFormat::RG16UI:	return nullptr;
			case Kaidel::TextureFormat::RG16NORM:	return nullptr;
			case Kaidel::TextureFormat::RG16I:	return nullptr;
			case Kaidel::TextureFormat::R32F:	RESIZE(float, data, iW, iH, resize_float, oW, oH, 1);
			case Kaidel::TextureFormat::R32UI:	RESIZE(uint32_t, data, iW, iH, resize_uint32, oW, oH, 1);
			case Kaidel::TextureFormat::R32I:	RESIZE(int32_t, data, iW, iH, resize_int32, oW, oH, 1);
			case Kaidel::TextureFormat::RG8:	RESIZE(uint8_t, data, iW, iH, resize_uint8_normalized, oW, oH, 2);
			case Kaidel::TextureFormat::RG8UI:	return nullptr;
			case Kaidel::TextureFormat::RG8NORM:	RESIZE(int8_t, data, iW, iH, resize_int8_normalized, oW, oH, 2);
			case Kaidel::TextureFormat::RG8I:	return nullptr;
			case Kaidel::TextureFormat::R16F:	return nullptr;
			case Kaidel::TextureFormat::R16UI:	return nullptr;
			case Kaidel::TextureFormat::R16NORM:	return nullptr;
			case Kaidel::TextureFormat::R16I:	return nullptr;
			case Kaidel::TextureFormat::R8:	RESIZE(uint8_t, data, iW, iH, resize_uint8_normalized, oW, oH, 1);
			case Kaidel::TextureFormat::R8UI:	return nullptr;
			case Kaidel::TextureFormat::R8NORM:	RESIZE(int8_t, data, iW, iH, resize_int8_normalized, oW, oH, 1);
			case Kaidel::TextureFormat::R8I:	return nullptr;
			}


		}

		
	}
}
