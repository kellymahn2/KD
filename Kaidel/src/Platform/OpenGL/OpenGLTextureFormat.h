#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Core.h"
#include <glad/glad.h>
namespace Kaidel {
	namespace Utils {
		GLenum KaidelTextureFormatToGLInternalFormat(TextureFormat format);
		GLenum KaidelTextureFormatToGLFormat(TextureFormat format);
		GLenum KaidelTextureFormatToGLValueFormat(TextureFormat format);
	}
}
