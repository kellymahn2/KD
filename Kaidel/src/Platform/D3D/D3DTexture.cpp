#include "KDpch.h"
#include "Platform/D3D/D3DTexture.h"
#include "Platform/D3D/D3DContext.h"
#include <stb_image.h>

namespace Kaidel {

	D3DTexture2D::D3DTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		D3D11_TEXTURE2D_DESC td{};
		td.Width = width;
		td.Height = height;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DYNAMIC;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		auto d3dContext = D3DContext::Get();
		D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&td, nullptr, &m_TexturePtr));
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Example filter mode
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Example addressing mode
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		D3DASSERT(d3dContext->GetDevice()->CreateSamplerState(&samplerDesc, &m_Params));
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
		srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvd.ViewDimension = D3D10_1_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MipLevels = 1;
		D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(m_TexturePtr, &srvd, &m_SRV));
	}
	D3DTexture2D::D3DTexture2D(const std::string& path)
		: m_Path(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		KD_CORE_ASSERT(data, "Failed to load image!");
		m_Width = width;
		m_Height = height;
		D3D11_TEXTURE2D_DESC td{};
		td.Width = m_Width;
		td.Height = m_Height;
		td.MipLevels = 1;
		td.ArraySize = 1;
		if (channels == 4)
			td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		else if (channels == 3)
			td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		td.SampleDesc.Count = 1;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		auto d3dContext = D3DContext::Get();
		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = data;
		initData.SysMemPitch = width * 4;
		D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&td, &initData, &m_TexturePtr));


		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Example filter mode
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Example addressing mode
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		D3DASSERT(d3dContext->GetDevice()->CreateSamplerState(&samplerDesc, &m_Params));
		stbi_image_free(data);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
		srvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvd.ViewDimension = D3D10_1_SRV_DIMENSION_TEXTURE2D;
		srvd.Texture2D.MipLevels = 1;
		D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(m_TexturePtr, &srvd, &m_SRV));
	}

	D3DTexture2D::~D3DTexture2D()
	{
		if (m_TexturePtr)
			m_TexturePtr->Release();
		if (m_Params)
			m_Params->Release();
		if (m_SRV)
			m_SRV->Release();
	}

	void D3DTexture2D::SetData(void* data, uint32_t size)
	{
		D3D11_MAPPED_SUBRESOURCE resourceMap{};
		auto d3dContext = D3DContext::Get();
		D3DASSERT(d3dContext->GetDeviceContext()->Map(m_TexturePtr, 0, D3D11_MAP_WRITE_DISCARD, 0, &resourceMap));
		for (int i = 0; i < m_Width; ++i) {
			for (int j = 0; j < m_Height; ++j) {
				((uint32_t*)resourceMap.pData)[j * m_Width + i] = ((uint32_t*)data)[j * m_Width + i];
			}
		}
		d3dContext->GetDeviceContext()->Unmap(m_TexturePtr, 0);
	}

	void D3DTexture2D::Bind(uint32_t slot) const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->PSSetShaderResources(slot, 1, &m_SRV);
		d3dContext->GetDeviceContext()->PSSetSamplers(slot, 1, &m_Params);
	}


	const std::string& D3DTexture2D::GetPath() const
	{
		return "";
	}

}
