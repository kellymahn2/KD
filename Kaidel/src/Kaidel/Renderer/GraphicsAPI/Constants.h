#pragma once


typedef unsigned int GLenum;

enum VkFormat;


namespace Kaidel {
	enum class TextureFormat {
		None = 0,

		RGBA32F,//4-component 128-bits
		RGBA32UI,//4-component 128-bits
		RGBA32I,//4-component 128-bits

		RGB32F,//3-component 96-bits
		RGB32UI,//3-component 96-bits
		RGB32I,//3-component 96-bits

		RGBA16F,//4-component 64-bits
		RGBA16UI,//4-component 64-bits
		RGBA16NORM,//4-component 64-bits
		RGBA16I,//4-component 64-bits

		RG32F,//2-component 64-bits
		RG32UI,//2-component 64-bits
		RG32I,//2-component 64-bits

		RGBA8,//4-component 32-bits
		RGBA8UI,//4-component 32-bits
		RGBA8NORM,//4-component 32-bits
		RGBA8I,//4-component 32-bits

		RG16F,//2-component 32-bits
		RG16UI,//2-component 32-bits
		RG16NORM,//2-component 32-bits
		RG16I,//2-component 32-bits

		R32F,//1-component 32-bits
		R32UI,//1-component 32-bits
		R32I,//1-component 32-bits

		RG8,//2-component 16-bits
		RG8UI,//2-component 16-bits
		RG8NORM,//2-component 16-bits
		RG8I,//2-component 16-bits

		R16F,//1-component 16-bits
		R16UI,//1-component 16-bits
		R16NORM,//1-component 16-bits
		R16I,//1-component 16-bits

		R8,//1-component 8-bits
		R8UI,//1-component 8-bits
		R8NORM,//1-component 8-bits
		R8I,//1-component 8-bits

		Depth16,
		Depth32F,
		Depth32,

		Depth24Stencil8

	};

	namespace Utils {
		GLenum KaidelTextureFormatToGLInternalFormat(TextureFormat format);
		GLenum KaidelTextureFormatToGLFormat(TextureFormat format);
		GLenum KaidelTextureFormatToGLValueFormat(TextureFormat format);
		VkFormat KaidelTextureFormatToVkFormat(TextureFormat format);

		bool IsDepthFormat(TextureFormat format);

	}

	struct TextureHandle;
	struct TextureArrayHandle;
	struct FramebufferAttachmentHandle;
}

