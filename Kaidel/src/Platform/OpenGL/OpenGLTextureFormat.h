#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Constants.h"
#include <glad/glad.h>
namespace Kaidel {
	namespace Utils {
		
		uint32_t KaidelTextureFormatComponentCount(TextureFormat format);
		uint32_t KaidelTextureFormatChannelSizeInBytes(TextureFormat format);
		uint32_t KaidelTextureFormatPixelSizeInBytes(TextureFormat format);
		void* ResizeTexture(void* data, TextureFormat iFormat, uint32_t iW, uint32_t iH, uint32_t oW, uint32_t oH);
	}
}
