#include "KDpch.h"
#include "D3DUniformBuffer.h"
#include "Platform/D3D/D3DContext.h"
#include "glad/glad.h"
namespace Kaidel {

	D3DUniformBuffer::D3DUniformBuffer(uint32_t size, uint32_t binding)
	{
		m_Binding = binding;
		D3D11_BUFFER_DESC bd{};
		bd.ByteWidth = size;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		auto d3dContext = D3DContext::Get();
		D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bd, nullptr, &m_UniformBuffer));
	}

	D3DUniformBuffer::~D3DUniformBuffer()
	{
		if (m_UniformBuffer)
			m_UniformBuffer->Release();
	}


	void D3DUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		D3D11_MAPPED_SUBRESOURCE mappedUB;
		auto d3dContext = D3DContext::Get();
		D3DASSERT(d3dContext->GetDeviceContext()->Map(m_UniformBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedUB));
		memcpy((void*)((uint64_t)mappedUB.pData+(uint64_t)offset), data, size);
		d3dContext->GetDeviceContext()->Unmap(m_UniformBuffer, 0);

	}

	void D3DUniformBuffer::Bind(uint32_t binding)
	{
		m_Binding = binding;
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->VSSetConstantBuffers(m_Binding, 1, &m_UniformBuffer);
		d3dContext->GetDeviceContext()->PSSetConstantBuffers(m_Binding, 1, &m_UniformBuffer);
	}

	void D3DUniformBuffer::Bind()
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->VSSetConstantBuffers(m_Binding, 1, &m_UniformBuffer);
		d3dContext->GetDeviceContext()->PSSetConstantBuffers(m_Binding, 1, &m_UniformBuffer);
	}

	void D3DUniformBuffer::UnBind()
	{
		auto d3dContext = D3DContext::Get();
		ID3D11Buffer* nullBuffer = nullptr;
		d3dContext->GetDeviceContext()->VSSetConstantBuffers(m_Binding, 1, &nullBuffer);
		d3dContext->GetDeviceContext()->PSSetConstantBuffers(m_Binding, 1, &nullBuffer);
	}

}
