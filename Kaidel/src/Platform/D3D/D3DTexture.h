#pragma once

#include "Kaidel/Renderer/Texture.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

#include <glad/glad.h>

namespace Kaidel {

	class D3DTexture2D : public Texture2D
	{
	public:
		D3DTexture2D(uint32_t width, uint32_t height);
		D3DTexture2D(const std::string& path);
		virtual ~D3DTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width;  }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint64_t GetRendererID() const override { return (uint64_t)m_SRV; }
		
		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == ((D3DTexture2D&)other).m_RendererID;
		}



		const std::string& GetPath() const override;

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;

		ID3D11Texture2D* m_TexturePtr;
		ID3D11SamplerState* m_Params;
		ID3D11ShaderResourceView* m_SRV;
	};

}
