#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

#include <glad/glad.h>

namespace Kaidel {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width;  }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint64_t GetRendererID() const override { return m_RendererID; }
		
		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
		}



		const std::string& GetPath() const override;

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
	class OpenGLTexture2DArray : public Texture2DArray {
	public:
		OpenGLTexture2DArray(uint32_t width, uint32_t height);
		virtual ~OpenGLTexture2DArray();
		
		virtual void SetTextureData(void* data, uint32_t width, uint32_t height, uint32_t index) override;
		virtual void Bind(uint32_t slot = 0)const override;
		virtual uint32_t PushTexture(void* data, uint32_t width, uint32_t height,bool shouldFlip)override;
		virtual uint32_t PushTexture(const std::string& src,bool shouldFlip)override;

	private:
		void ResizeTextureArray(uint32_t newLayerCount);
		uint32_t PushLoadedTexture(void* data);
		void* ScaleImage(void* orgImage, uint32_t orgWidth, uint32_t orgHeight, uint32_t newWidth, uint32_t newHeight);
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height, m_Depth;
		uint32_t m_SetCount = 0;
		std::unordered_map<std::string, uint32_t> m_LoadedTextures;
	};
	class OpenGLDepth2DArray : public Depth2DArray {
	public:
		OpenGLDepth2DArray(uint32_t width, uint32_t height);
		virtual ~OpenGLDepth2DArray();

		virtual void Bind(uint32_t slot = 0)const override ;
		virtual uint32_t PushDepth(uint32_t width, uint32_t height)override;
		virtual void ClearLayer(uint32_t index, float value)override;
		uint32_t GetRendererID()const override { return m_RendererID; }
		void* GetData();
		void PopDepth() { --m_SetCount; }
		uint32_t GetSize()const { return m_SetCount; }
	private:
		void ResizeTextureArray(uint32_t newLayerCount);
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height, m_Depth;
		uint32_t m_SetCount = 0;
	};

}
