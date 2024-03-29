#include "KDpch.h"
#include "OpenGLCubeMap.h"
#include "OpenGLTextureFormat.h"
#include <stb_image.h>
namespace Kaidel {


	namespace Utils {
		static GLenum KaidelTextureSideToGLTextureDirection(CubeMapSide side) {
			switch (side)
			{
			case Kaidel::CubeMapSide::Left:return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			case Kaidel::CubeMapSide::Right: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
			case Kaidel::CubeMapSide::Top: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			case Kaidel::CubeMapSide::Bottom: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			case Kaidel::CubeMapSide::Front:return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			case Kaidel::CubeMapSide::Back:return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			}
		}
		static bool NeedsResize(uint32_t w, uint32_t h, uint32_t expectedWidth, uint32_t expectedHeight) { return w != expectedWidth || h != expectedHeight; }


		static void SetTexture(uint32_t texture, uint32_t sideIndex, TextureFormat format, uint32_t width, uint32_t height, void* data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + sideIndex, 0, Utils::KaidelTextureFormatToGLInternalFormat(format), width, height,
				GL_FALSE, Utils::KaidelTextureFormatToGLFormat(format), Utils::KaidelTextureFormatToGLValueFormat(format), data);
		}

	}



	OpenGLCubeMap::OpenGLCubeMap(uint32_t width, uint32_t height, TextureFormat format)
		:m_Width(width), m_Height(height), m_TextureFormat(format)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_RendererID);

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	}
	
	OpenGLCubeMap::~OpenGLCubeMap() {
		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGLCubeMap::SetWithResize(CubeMapSide side, void* data, uint32_t width, uint32_t height){
		void* sclImage = Utils::ResizeTexture(data, m_TextureFormat, width, height, m_Width, m_Height);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		Utils::SetTexture(m_RendererID, (uint32_t)side, m_TextureFormat, m_Width, m_Height, sclImage);
		delete[] sclImage;
	}
	void OpenGLCubeMap::Set(CubeMapSide side, void* data){
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		Utils::SetTexture(m_RendererID, (uint32_t)side, m_TextureFormat, m_Width, m_Height, data);
	}
	void OpenGLCubeMap::AllWithResize(void* data, uint32_t width, uint32_t height){
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		void* sclImage = Utils::ResizeTexture(data, m_TextureFormat, width, height, m_Width, m_Height);
		for (uint32_t i = 0; i < 6; ++i) {
			Utils::SetTexture(m_RendererID, i, m_TextureFormat, m_Width, m_Height, sclImage);
		}
		delete[] sclImage;
	}

	void OpenGLCubeMap::All(void* data) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		for (uint32_t i = 0; i < 6; ++i) {
			Utils::SetTexture(m_RendererID, i, m_TextureFormat, m_Width, m_Height, data);
		}
	}

	void OpenGLCubeMap::SetData(CubeMapSide side, void* data, uint32_t width, uint32_t height) {
		Utils::NeedsResize(width, height, m_Width, m_Height)? SetWithResize(side, data, width,height) : Set(side,data);
	}

	void OpenGLCubeMap::SetData(CubeMapSide side, const std::string& src, bool shouldFlip){
		int w, h, num_channels;
		stbi_set_flip_vertically_on_load(shouldFlip);
		uint8_t* img = stbi_load(src.c_str(), &w, &h, &num_channels, Utils::KaidelTextureFormatComponentCount(m_TextureFormat));

		Utils::NeedsResize(w, h, m_Width, m_Height) ? SetWithResize(side, img, w, h) : Set(side,img);
		stbi_image_free(img);
	}
	void OpenGLCubeMap::SetAll(const std::string& src, bool shouldFlip) {
		int w, h, num_channels;
		stbi_set_flip_vertically_on_load(shouldFlip);
		uint8_t* img = stbi_load(src.c_str(), &w, &h, &num_channels, Utils::KaidelTextureFormatComponentCount(m_TextureFormat));

		Utils::NeedsResize(w, h, m_Width, m_Height) ? AllWithResize(img, w, h) : All(img);
		stbi_image_free(img);
	}
	
	void OpenGLCubeMap::SetAll(void* data, uint32_t width, uint32_t height) {
		Utils::NeedsResize(width, height, m_Width, m_Height) ? AllWithResize(data, width, height) : All(data);
	}

	void OpenGLCubeMap::Bind(uint32_t slot)const {
		glBindTextureUnit(slot, m_RendererID);
	}
}
