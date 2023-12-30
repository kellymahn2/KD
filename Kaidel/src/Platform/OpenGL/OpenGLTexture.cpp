#include "KDpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"

#include <stb_image.h>
#include <stb_image_resize.h>

namespace Kaidel {

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		KD_PROFILE_FUNCTION();

		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		KD_PROFILE_FUNCTION();

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			KD_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		KD_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		KD_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		KD_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		KD_PROFILE_FUNCTION();

		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		KD_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		KD_PROFILE_FUNCTION();

		glBindTextureUnit(slot, m_RendererID);
	}


	const std::string& OpenGLTexture2D::GetPath() const
	{
		return m_Path;
	}





	OpenGLTexture2DArray::OpenGLTexture2DArray(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height), m_Depth(2)
	{
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, width, height, m_Depth);
	}
	OpenGLTexture2DArray::~OpenGLTexture2DArray() {
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTexture2DArray::SetTextureData(void* data, uint32_t width, uint32_t height, uint32_t index) {
		KD_CORE_ASSERT(index < m_Depth);
		void* img = data;
		if (width != m_Width || height != m_Height) {
			img = ScaleImage(data, width, height, m_Width, m_Height);
		}
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, m_Width, m_Height, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
		if (width != m_Width || height != m_Height) {
			delete img;
		}
	}
	void OpenGLTexture2DArray::Bind(uint32_t slot)const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
	}
	uint32_t OpenGLTexture2DArray::PushTexture(void* data, uint32_t width, uint32_t height) {
		if (m_SetCount + 1 > m_Depth) {
			ResizeTextureArray(m_SetCount * 2);
		}
		void* img = data;
		if (width != m_Width || height != m_Height) {
			img = ScaleImage(data, width, height, m_Width, m_Height);
		}
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, m_SetCount, m_Width, m_Height, 1, GL_RGBA, GL_UNSIGNED_BYTE, img);
		m_SetCount++;

		if (width != m_Width || height != m_Height) {
			delete img;
		}
		return m_SetCount - 1;
	}
	uint32_t OpenGLTexture2DArray::PushTexture(const std::string& src) {
		if (m_LoadedTextures.find(src) != m_LoadedTextures.end()) {
			return m_LoadedTextures.at(src);
		}
		void* data = LoadImageScaled(src, m_Width, m_Height);
		uint32_t index = PushTexture(data, m_Width,m_Height);
		stbi_image_free(data);
		m_LoadedTextures[src] = index;
		return index;
	}
	void OpenGLTexture2DArray::ResizeTextureArray(uint32_t newLayerCount) {
		std::vector<uint8_t> existingData(m_Width * m_Width * m_Depth * 4);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, existingData.data());
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &m_RendererID);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, m_Width, m_Height, newLayerCount);

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, m_Width, m_Height,m_SetCount, GL_RGBA, GL_UNSIGNED_BYTE, existingData.data());
		m_Depth = newLayerCount;

	}

	void* OpenGLTexture2DArray::LoadImageScaled(const std::string& path, uint32_t width, uint32_t height)
	{
		int w, h, channels;
		stbi_set_flip_vertically_on_load(1);
		void* orgImage = stbi_load(path.c_str(), &w, &h, &channels, 4);
		KD_CORE_ASSERT(orgImage, "Failed to load image");
		if (w != width || h != height) {
			void* scldImage = ScaleImage(orgImage, w, h, width, height);
			stbi_image_free(orgImage);
			return scldImage;
		}
		return orgImage;
	}

	void* OpenGLTexture2DArray::ScaleImage(void* orgImage, uint32_t orgWidth, uint32_t orgHeight, uint32_t newWidth, uint32_t newHeight)
	{
		uint8_t* scldImage = new uint8_t[newWidth * newHeight * 4];
		KD_CORE_ASSERT(stbir_resize_uint8((const uint8_t*)orgImage, orgWidth, orgHeight, 0, scldImage, newWidth, newHeight, 0, 4)==1
			,"Failed to scale image from ({},{}) to ({},{})",orgWidth,orgHeight,newWidth,newHeight);
		return scldImage;
	}




	OpenGLDepth2DArray::OpenGLDepth2DArray(uint32_t width, uint32_t height)
		:m_Width(width), m_Height(height), m_Depth(2)
	{
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, width, height, m_Depth);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	OpenGLDepth2DArray::~OpenGLDepth2DArray(){
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLDepth2DArray::ClearLayer(uint32_t index, float value){
		glClearTexImage(m_RendererID, index, GL_DEPTH_COMPONENT32, GL_FLOAT, &value);
	}
	uint32_t OpenGLDepth2DArray::PushDepth(uint32_t width, uint32_t height){
		if (m_SetCount + 1 > m_Depth) {
			ResizeTextureArray(m_SetCount * 2);
		}
		m_SetCount++;
		return m_SetCount - 1;
	}
	void OpenGLDepth2DArray::ResizeTextureArray(uint32_t newLayerCount) {
		glDeleteTextures(1, &m_RendererID);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, m_Width, m_Height, newLayerCount);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		m_Depth = newLayerCount;
	}
}
