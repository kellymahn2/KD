#pragma once
#include "Kaidel/Renderer/GraphicsAPI/TextureArray.h"

#include <glad/glad.h>
namespace Kaidel {




	class OpenGLTexture2DArray : public Texture2DArray {
	public:
		OpenGLTexture2DArray(uint32_t width, uint32_t height, TextureFormat textureFormat,bool loadsSpecs = true);
		virtual ~OpenGLTexture2DArray();

		virtual void SetTextureData(void* data, uint32_t width, uint32_t height, uint32_t index) override {};
		virtual void Bind(uint32_t slot = 0)const override;
		virtual uint32_t PushTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip)override;
		virtual uint32_t PushTexture(const std::string& src, bool shouldFlip)override;

		virtual void InsertTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip = true) override;
		virtual void InsertTexture(uint32_t slot, const std::string& src, bool shouldFlip = true) override;

		virtual void InsertOrReplaceTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip = true) override;
		virtual void InsertOrReplaceTexture(uint32_t slot, const std::string& src, bool shouldFlip = true) override;


		virtual void ReplaceTexture(uint32_t slot, void* data, uint32_t width, uint32_t height, bool shouldFlip = true)override;
		virtual void ReplaceTexture(uint32_t slot, const std::string& src, bool shouldFlip = true) override;


		virtual TextureFormat GetFormat()const override { return m_TextureFormat; }

		virtual TextureArrayHandle GetHandle(uint32_t index) const override;

		virtual Ref<TextureView> GetView(uint32_t index)override;

		virtual uint32_t GetWidth()const { return m_Width; }
		virtual uint32_t GetHeight()const { return m_Height; }
		virtual uint32_t GetLayerCount()const { return m_SetCount; }
		virtual uint32_t GetCapacity()const {return m_Depth;}

		virtual void ClearLayer(uint32_t slot, const float* data)const override;
		virtual void Clear(const float* data) const override;

		virtual const std::unordered_map<std::string, uint32_t>& GetLoadedTextures() override { return {}; }

		virtual const std::vector<ArraySlotSpecification>& GetSpecifications()const override { return m_Specifications; }


	private:
		void ResizeTextureArray(uint32_t newLayerCount);
		void InsertSpec(const ArraySlotSpecification& spec,uint32_t slot);
		void InsertView(uint32_t slot);
	private:
		void PushWithResize(void* data, uint32_t width, uint32_t height);
		void Push(void* data);
	private:
		void ReplaceWithResize(void* data, uint32_t width, uint32_t height, uint32_t slot);
		void Replace(void* data,uint32_t slot);
	private:
		void InsertWithResize(void* data, uint32_t width, uint32_t height, uint32_t slot);
		void Insert(void* data, uint32_t slot);
	private:
		void RecreateViews(uint32_t start,uint32_t end = -1);
		void RecreateViews();
	private:

		uint32_t m_RendererID;
		uint32_t m_Width, m_Height, m_Depth;
		uint32_t m_SetCount = 0;

		TextureFormat m_TextureFormat;

		std::vector<Ref<TextureView>> m_GeneratedViews;

		std::vector<ArraySlotSpecification> m_Specifications;
		bool m_LoadsSpecs;

		friend class OpenGLTexture2DView;
		friend class OpenGLFramebuffer;
		friend class OpenGLTexture2D;
		friend class OpenGLTextureCopier;
		friend class OpenGLTextureView;
	};
}
