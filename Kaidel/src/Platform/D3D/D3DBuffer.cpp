#include "KDpch.h"
#include "Platform/D3D/D3DBuffer.h"
#include "Platform/D3D/D3DContext.h"
#include <glad/glad.h>

namespace Kaidel {

	/////////////////////////////////////////////////////////////////////////////
	// VertexBuffer /////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////

	D3DVertexBuffer::D3DVertexBuffer(uint32_t size)
		:m_MaxSize(size)
	{
		auto d3dContext = D3DContext::Get();
		
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.ByteWidth = size?size:4;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bd, NULL, &m_VertexBuffer));
	}

	D3DVertexBuffer::D3DVertexBuffer(float* vertices, uint32_t size)
		:m_MaxSize(size)
	{
		auto d3dContext = D3DContext::Get();
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.ByteWidth = size;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vertexData;
		ZeroMemory(&vertexData, sizeof(D3D11_SUBRESOURCE_DATA));
		vertexData.pSysMem = vertices;
		D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bd, &vertexData, &m_VertexBuffer));
	}

	D3DVertexBuffer::~D3DVertexBuffer()
	{
		if (m_VertexBuffer)
			m_VertexBuffer->Release();
	}

	void D3DVertexBuffer::Bind() const
	{
		auto d3dContext = D3DContext::Get();
		uint32_t stride = m_Layout.GetStride();
		uint32_t offset = 0;
		d3dContext->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	}

	void D3DVertexBuffer::Unbind() const
	{
		auto d3dContext = D3DContext::Get();
		ID3D11Buffer* nullBuffer = nullptr;
		uint32_t stride = m_Layout.GetStride();
		uint32_t offset = 0;
		d3dContext->GetDeviceContext()->IASetVertexBuffers(0,1, &nullBuffer, &stride, &offset);
	}

	void D3DVertexBuffer::SetData(const void* data, uint32_t size)
	{
		auto d3dContext = D3DContext::Get();
		if (m_MaxSize < size) {
			m_MaxSize = size * 2.0f;
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
			bd.ByteWidth = m_MaxSize;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.MiscFlags = 0;
			ID3D11Buffer* newVB = nullptr;
			D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bd, nullptr,&newVB));
			d3dContext->GetDeviceContext()->CopySubresourceRegion(newVB, 0, 0, 0, 0, m_VertexBuffer, 0, nullptr);
			m_VertexBuffer->Release();
			m_VertexBuffer = newVB;
		}
		D3D11_MAPPED_SUBRESOURCE mappedVB{};
		D3DASSERT(d3dContext->GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB));
		memcpy(mappedVB.pData,data,size);
		d3dContext->GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	}

	void D3DVertexBuffer::ResetBuffer(uint32_t newSize)
	{
	}

	/////////////////////////////////////////////////////////////////////////////
	// IndexBuffer //////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////

	D3DIndexBuffer::D3DIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		auto d3dContext = D3DContext::Get();
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.ByteWidth = count * sizeof(uint32_t);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA indexData;
		ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
		indexData.pSysMem = indices;
		D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bd, &indexData, &m_IndexBuffer));
	}

	D3DIndexBuffer::~D3DIndexBuffer()
	{
		if (m_IndexBuffer)
			m_IndexBuffer->Release();
	}

	void D3DIndexBuffer::Bind() const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	}

	void D3DIndexBuffer::Unbind() const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	}


}
