#include "KDpch.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "Kaidel/Core/Timer.h"
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

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);



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
	//TODO: Fix this not valid.
	void OpenGLTexture2DArray::Bind(uint32_t slot)const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
	}

	uint32_t OpenGLTexture2DArray::PushLoadedTexture(void* data) {
		if (m_SetCount + 1 > m_Depth) {
			Timer timer(("Resizing"));
			ResizeTextureArray((m_SetCount * 2));
		}
		Timer timer("Pushing Texture");
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, m_SetCount, m_Width, m_Height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		return m_SetCount++;
	}
	uint32_t OpenGLTexture2DArray::PushTexture(void* data, uint32_t width, uint32_t height,bool shouldFlip) {
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
			uint8_t* sclImage = new uint8_t[m_Width*m_Height*4];
			KD_CORE_ASSERT(stbir_resize_uint8(data, w, h, 0, sclImage, m_Width, m_Height, 0, 4) == 1);
			stbi_image_free(data);
			uint32_t index  = PushLoadedTexture(sclImage);
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
		/*std::vector<uint8_t> existingData(m_Width * m_Width * m_Depth * 4);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, GL_UNSIGNED_BYTE, existingData.data());
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &m_RendererID);*/

		//glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_RendererID);

		uint32_t newTexture = 0;
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &newTexture);
		glBindTexture(GL_TEXTURE_2D_ARRAY, newTexture);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, m_Width, m_Height, newLayerCount);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);

		glCopyImageSubData(m_RendererID, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, newTexture,GL_TEXTURE_2D_ARRAY , 0, 0, 0, 0, m_Width, m_Height, m_SetCount);
		
		
		glDeleteTextures(1, &m_RendererID);
		m_RendererID = newTexture;
		//glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, m_Width, m_Height,m_SetCount, GL_RGBA, GL_UNSIGNED_BYTE, existingData.data());
		m_Depth = newLayerCount;

	}
	void* OpenGLTexture2DArray::ScaleImage(void* orgImage, uint32_t orgWidth, uint32_t orgHeight, uint32_t newWidth, uint32_t newHeight)
	{
		Timer timer(fmt::format("Scaling From {},{} To {},{}", orgWidth, orgHeight, newWidth, newHeight));
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}
	OpenGLDepth2DArray::~OpenGLDepth2DArray(){
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLDepth2DArray::Bind(uint32_t slot)const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
	}
	void OpenGLDepth2DArray::ClearLayer(uint32_t index, float value){
		glClearTexSubImage(m_RendererID, 0, 0, 0, index, m_Width, m_Height, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &value);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		
		m_Depth = newLayerCount;
	}

	void* OpenGLDepth2DArray::GetData() {
		float* f = new float[m_Width * m_Height*m_Depth];
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_RendererID);
		glGetTexImage(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, f);
		return f;
	}
}
