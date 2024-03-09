#include "KDpch.h"
#include "OpenGLTextureArray.h"
#include "OpenGLTextureFormat.h"
#include <stb_image.h>
#include <stb_image_resize.h>

namespace Kaidel {


	namespace Utils {
		static GLint CreateTextureArray(uint32_t width,uint32_t height,uint32_t depth,TextureFormat format) {
			uint32_t texture = 0;
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texture);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, Utils::KaidelTextureFormatToGLInternalFormat(format), width, height, depth);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
			return texture;
		}

	}



	OpenGLTexture2DArray::OpenGLTexture2DArray(uint32_t width, uint32_t height, TextureFormat textureFormat)
		:m_Width(width), m_Height(height), m_Depth(2),m_TextureFormat(textureFormat)
	{
		m_RendererID = Utils::CreateTextureArray(m_Width, m_Height, m_Depth, m_TextureFormat);
		

	}
	OpenGLTexture2DArray::~OpenGLTexture2DArray() {
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGLTexture2DArray::Bind(uint32_t slot)const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
	}

	uint32_t OpenGLTexture2DArray::PushLoadedTexture(void* data) {
		if (m_SetCount + 1 > m_Depth) {
			ResizeTextureArray((m_SetCount * 2));
		}
		if (data != nullptr) {
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, m_SetCount, m_Width, m_Height, 1, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_UNSIGNED_BYTE, data);
		}
		return m_SetCount++;
	}
	uint32_t OpenGLTexture2DArray::PushTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip) {
		if (width != m_Width || height != m_Height) {
			uint8_t* sclImage = new uint8_t[m_Width * m_Height * 4];
			KD_CORE_ASSERT(stbir_resize_uint8((uint8_t*)data, width, height, 0, sclImage, m_Width, m_Height, 0, 4) == 1);
			uint32_t index = PushLoadedTexture(sclImage);
			delete[] sclImage;
			return index;
		}
		return PushLoadedTexture(data);
	}
	uint32_t OpenGLTexture2DArray::PushTexture(const std::string& src, bool shouldFlip) {
		if (m_LoadedTextures.find(src) != m_LoadedTextures.end()) {
			return m_LoadedTextures.at(src);
		}

		int w, h, channels;
		uint8_t* data;
		{
			Timer timer("Image Loading");
			stbi_set_flip_vertically_on_load(shouldFlip);
			data = stbi_load(src.c_str(), &w, &h, &channels, 4);
		}
		KD_CORE_ASSERT(data);
		if (w != m_Width || h != m_Height) {
			Timer timer(fmt::format("Scaling image from {},{} To {},{}", w, h, m_Width, m_Height));
			uint8_t* sclImage = new uint8_t[m_Width * m_Height * 4];
			KD_CORE_ASSERT(stbir_resize_uint8(data, w, h, 0, sclImage, m_Width, m_Height, 0, 4) == 1);
			stbi_image_free(data);
			uint32_t index = PushLoadedTexture(sclImage);
			delete[] sclImage;
			m_LoadedTextures[src] = index;
			return index;
		}

		uint32_t index = PushLoadedTexture(data);
		stbi_image_free(data);
		m_LoadedTextures[src] = index;
		return index;
	}
	void OpenGLTexture2DArray::ResizeTextureArray(uint32_t newLayerCount) {
		uint32_t newTexture = Utils::CreateTextureArray(m_Width, m_Height, newLayerCount, m_TextureFormat);
		glCopyImageSubData(m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, newTexture, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, m_Width, m_Height, m_SetCount);

		glDeleteTextures(1, &m_RendererID);
		m_RendererID = newTexture;
		m_Depth = newLayerCount;

		for (auto& [index, view] : m_GeneratedViews) {
			view = TextureView::Create(GetHandle(index));
		}

	}
	void* OpenGLTexture2DArray::ScaleImage(void* orgImage, uint32_t orgWidth, uint32_t orgHeight, uint32_t newWidth, uint32_t newHeight)
	{
		Timer timer(fmt::format("Scaling From {},{} To {},{}", orgWidth, orgHeight, newWidth, newHeight));
		uint8_t* scldImage = new uint8_t[newWidth * newHeight * 4];
		KD_CORE_ASSERT(stbir_resize_uint8((const uint8_t*)orgImage, orgWidth, orgHeight, 0, scldImage, newWidth, newHeight, 0, 4) == 1
			, "Failed to scale image from ({},{}) to ({},{})", orgWidth, orgHeight, newWidth, newHeight);
		return scldImage;
	}

	TextureArrayHandle OpenGLTexture2DArray::GetHandle(uint32_t index) const {
		TextureArrayHandle handle{};
		handle.Array = const_cast<OpenGLTexture2DArray*>(this);
		handle.SlotIndex = index;
		return handle;
	}

	Ref<TextureView> OpenGLTexture2DArray::GetView(uint32_t index) {
		{
			auto it = m_GeneratedViews.find(index);
			if (it != m_GeneratedViews.end())
				return it->second;
		}
		return m_GeneratedViews[index] = TextureView::Create(GetHandle(index));
	}

	void OpenGLTexture2DArray::ClearLayer(uint32_t slot, const float* data)const {
		glClearTexSubImage(m_RendererID, 0, 0, 0, slot, m_Width, m_Height, 1, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_FLOAT, data);
	}
	void OpenGLTexture2DArray::Clear(const float* data) const{
		glClearTexImage(m_RendererID, 0, Utils::KaidelTextureFormatToGLFormat(m_TextureFormat), GL_FLOAT, data);
	}

}
