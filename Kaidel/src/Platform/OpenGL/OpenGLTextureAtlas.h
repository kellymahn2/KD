#pragma once
#include "Kaidel/Renderer/TextureAtlas.h"
#include <glad/glad.h>
namespace Kaidel {

	struct OpenGLSubTexture2D : public SubTexture2D {
	public:
		void SetTextureCoordinates(float texCoords[4][2]) override;
		float* GetTextureCoordinates(uint32_t index) const override;
	private:
		float m_TextureCoordinates[4][2] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };


	};
	class OpenGLTextureAtlas2D : public TextureAtlas2D {
	public:
		OpenGLTextureAtlas2D() = default;
		OpenGLTextureAtlas2D(const std::string& path);
		~OpenGLTextureAtlas2D();

		uint32_t GetWidth() const override;


		uint32_t GetHeight() const override;


		uint32_t GetRendererID() const override;


		const std::string& GetPath() const override;


		void Bind(uint32_t slot = 0) const override;


		bool operator ==(const TextureAtlas2D& other) const override;


		Ref<SubTexture2D> GetSubTexture(uint32_t x, uint32_t y) override;


		void SetData(void* data, uint32_t size) override;


		bool operator ==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}

	private:
		void CreateSubTextures();
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
		uint32_t m_MaxXIndex;
		std::vector<Ref<SubTexture2D>> m_Textures;
	};

}
