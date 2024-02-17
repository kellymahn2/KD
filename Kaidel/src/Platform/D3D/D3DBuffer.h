#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
namespace Kaidel {

	class D3DVertexBuffer : public VertexBuffer
	{
	public:
		D3DVertexBuffer(uint32_t size);
		D3DVertexBuffer(float* vertices, uint32_t size);
		virtual ~D3DVertexBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		
		virtual void SetData(const void* data, uint32_t size) override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	private:
		void ResetBuffer(uint32_t newSize);
	private:
		ID3D11Buffer* m_VertexBuffer;
		uint32_t m_RendererID;
		size_t m_MaxSize;
		BufferLayout m_Layout;
		friend class D3DVertexArray;
	};

	class D3DIndexBuffer : public IndexBuffer
	{
	public:
		D3DIndexBuffer(uint32_t* indices, uint32_t count);
		virtual ~D3DIndexBuffer();

		virtual void Bind() const;
		virtual void Unbind() const;

		virtual uint32_t GetCount() const { return m_Count; }
	private:
		ID3D11Buffer* m_IndexBuffer;
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

	

}
