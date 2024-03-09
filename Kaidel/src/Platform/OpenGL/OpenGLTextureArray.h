#pragma once
#include "Kaidel/Renderer/GraphicsAPI/TextureArray.h"

#include <glad/glad.h>
namespace Kaidel {

	class OpenGLTexture2DArray : public Texture2DArray {
	public:
		OpenGLTexture2DArray(uint32_t width, uint32_t height,TextureFormat textureFormat);
		virtual ~OpenGLTexture2DArray();

		virtual void SetTextureData(void* data, uint32_t width, uint32_t height, uint32_t index) override {};
		virtual void Bind(uint32_t slot = 0)const override;
		virtual uint32_t PushTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip)override;
		virtual uint32_t PushTexture(const std::string& src, bool shouldFlip)override;

		virtual TextureFormat GetFormat()const override { return m_TextureFormat; }

		virtual TextureArrayHandle GetHandle(uint32_t index) const override;

		virtual Ref<TextureView> GetView(uint32_t index)override;

		virtual void ClearLayer(uint32_t slot, const float* data)const override;
		virtual void Clear(const float* data) const override;

	private:
		void ResizeTextureArray(uint32_t newLayerCount);
		uint32_t PushLoadedTexture(void* data);
		void* ScaleImage(void* orgImage, uint32_t orgWidth, uint32_t orgHeight, uint32_t newWidth, uint32_t newHeight);
	private:

		uint32_t m_RendererID;
		uint32_t m_Width, m_Height, m_Depth;
		uint32_t m_SetCount = 0;
		std::unordered_map<std::string, uint32_t> m_LoadedTextures;
		TextureFormat m_TextureFormat;

		std::unordered_map<uint32_t, Ref<TextureView>> m_GeneratedViews;

		friend class OpenGLTexture2DView;
		friend class OpenGLFramebuffer;
		friend class OpenGLTexture2D;
		friend class OpenGLTextureCopier;
		friend class OpenGLTextureView;
	};
}
