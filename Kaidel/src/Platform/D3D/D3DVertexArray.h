#pragma once

#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Shader.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
namespace Kaidel {

	class D3DVertexArray : public VertexArray
	{
	public:
		D3DVertexArray();
		D3DVertexArray(Ref<Shader> shader);
		virtual ~D3DVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;

		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
		static D3DVertexArray* GetCurrentBound();
	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		ID3D11InputLayout* m_InputLayout;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Shader> m_Shader;

	};

}
