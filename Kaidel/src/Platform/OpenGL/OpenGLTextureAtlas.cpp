#include "KDpch.h"
#include "OpenGLTextureAtlas.h"
#include "stb_image.h"
namespace Kaidel {



	OpenGLTextureAtlas2D::OpenGLTextureAtlas2D(const std::string& path)
		:m_Path(path)
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
		CreateSubTextures();
	}


	OpenGLTextureAtlas2D::~OpenGLTextureAtlas2D()
	{
		KD_PROFILE_FUNCTION();
		glDeleteTextures(1, &m_RendererID);
	}


	uint32_t OpenGLTextureAtlas2D::GetWidth() const
	{
		return m_Width;
	}


	uint32_t OpenGLTextureAtlas2D::GetHeight() const
	{
		return m_Height;
	}


	uint32_t OpenGLTextureAtlas2D::GetRendererID() const
	{
		return m_RendererID;
	}


	const std::string& OpenGLTextureAtlas2D::GetPath() const
	{
		return m_Path;
	}


	void OpenGLTextureAtlas2D::Bind(uint32_t slot /*= 0*/) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}


	Kaidel::Ref<Kaidel::SubTexture2D> OpenGLTextureAtlas2D::GetSubTexture(uint32_t x, uint32_t y)
	{
		return m_Textures[y * m_MaxXIndex + x];
	}


	void OpenGLTextureAtlas2D::SetData(void* data, uint32_t size)
	{
	}

	void OpenGLTextureAtlas2D::CreateSubTextures()
	{
		constexpr float width = 18.0f;
		constexpr float height = 18.0f;
		uint32_t maxXIndex = (float)m_Width / width;
		uint32_t maxYIndex = (float)m_Height/ height;


		for (int j = 0; j < maxYIndex; ++j) {
			for (int i = 0; i < maxXIndex; ++i) {
				Ref<SubTexture2D> subTexture = Texture2D::CreateSubTexture();
				auto left = i * width/ (float)m_Width;
				auto right = (i + 1) * width/ (float)m_Width;
				auto top = (float)m_Height-j * height;
				auto bottom = (float)m_Height-(j+1 )* height;
				top /= (float)m_Height;
				bottom /= (float)m_Height;
				float coords[4][2] = { {left,bottom},{right,bottom},{right,top},{left,top} };
				subTexture->SetTextureCoordinates(coords);

				m_Textures.push_back(subTexture);
			}
		}
		m_MaxXIndex = maxXIndex;

	}

	bool OpenGLTextureAtlas2D::operator==(const TextureAtlas2D& other) const
	{
		return m_RendererID == other.GetRendererID();
	}


	void OpenGLSubTexture2D::SetTextureCoordinates(float texCoords[4][2])
	{
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 2; ++j) {
				m_TextureCoordinates[i][j] = texCoords[i][j];
			}
		}
	}


	float* OpenGLSubTexture2D::GetTextureCoordinates(uint32_t index) const
	{
		return (float*)(m_TextureCoordinates[index]);
	}

}
