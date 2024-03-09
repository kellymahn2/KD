#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

#include <glad/glad.h>

namespace Kaidel {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height,TextureFormat format);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width;  }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint64_t GetRendererID() const override { return m_RendererID; }
		
		virtual void SetData(void* data, uint32_t size) override;
		virtual void Reset(void* data, uint32_t width, uint32_t height)override;
		virtual void Bind(uint32_t slot = 0) const override;


		virtual TextureFormat GetFormat()const override { return m_TextureFormat; }

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}

		virtual TextureHandle GetHandle()const override;

		const std::string& GetPath() const override;

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		TextureFormat m_TextureFormat;

		friend class OpenGLFramebuffer;
		friend class OpenGLTexture2DArray;
		friend class OpenGLTextureCopier;
		friend class OpenGLTextureView;
	};

	

	
	

}
