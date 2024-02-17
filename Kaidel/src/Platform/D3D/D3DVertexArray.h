#pragma once

#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
namespace Kaidel {

	class D3DVertexArray : public VertexArray
	{
	public:
		D3DVertexArray(const VertexArraySpecification& spec);
		virtual ~D3DVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;


		virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_Specification.VertexBuffers; }
		virtual const Ref<IndexBuffer>& GetIndexBuffer() const { return m_Specification.IndexBuffer; }
		static D3DVertexArray* GetCurrentBound();
	private:
		ID3D11InputLayout* m_InputLayout;
		VertexArraySpecification m_Specification;
	};

}
