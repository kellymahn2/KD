#pragma once
#include <Kaidel/Renderer/GraphicsAPI/CubeMap.h>


#include <glad/glad.h>



namespace Kaidel {
	class OpenGLCubeMap : public CubeMap {
	public:

		OpenGLCubeMap(uint32_t width, uint32_t height, TextureFormat format);

		~OpenGLCubeMap();

		virtual void Bind(uint32_t slot = 0)const override;
		virtual void SetData(CubeMapSide side, void* data, uint32_t width, uint32_t height) override;
		virtual void SetData(CubeMapSide side, const std::string& src, bool shouldFlip)override;
		virtual void SetAll(const std::string& src, bool shouldFlip) override;
		virtual void SetAll(void* data, uint32_t width, uint32_t height) override;
		virtual uint32_t GetWidth()const override {return m_Width;}
		virtual uint32_t GetHeight()const override { return m_Height; }
	private:
		void SetWithResize(CubeMapSide side,void* data, uint32_t width, uint32_t height);
		void Set(CubeMapSide side, void* data);

		void AllWithResize(void* data, uint32_t width, uint32_t height);
		void All(void* data);

	private:
		uint32_t m_RendererID;
		uint32_t m_Width;
		uint32_t m_Height;
		TextureFormat m_TextureFormat;
	};
}
