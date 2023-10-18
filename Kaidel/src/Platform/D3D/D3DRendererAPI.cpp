#include "KDpch.h"
#include "Platform/D3D/D3DRendererAPI.h"
#include "Platform/D3D/D3DContext.h"
#include <glad/glad.h>

namespace Kaidel {
	

	void D3DRendererAPI::Init()
	{
	}

	void D3DRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->RecreateSwapChain(width, height);
		D3D11_VIEWPORT viewport;
		UINT num = 1;
		d3dContext->GetDeviceContext()->RSGetViewports(&num, &viewport);
		viewport.TopLeftX = x;
		viewport.TopLeftY = y;
		viewport.Width = width;
		viewport.Height = height;
		d3dContext->GetDeviceContext()->RSSetViewports(1, &viewport);
	}

	void D3DRendererAPI::SetClearColor(const glm::vec4& color)
	{
		m_ClearColor = color;
	}

	void D3DRendererAPI::Clear()
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->ClearRenderTargetView(d3dContext->GetBackBuffer(), &m_ClearColor[0]);
		d3dContext->GetDeviceContext()->ClearDepthStencilView(d3dContext->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void D3DRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		auto d3dContext = D3DContext::Get();
		try {
			uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
			d3dContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			d3dContext->GetDeviceContext()->DrawIndexed(count, 0, 0);

		}
		catch (std::exception& e) {
			KD_CORE_ASSERT(false);
		}

	}
	void D3DRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		d3dContext->GetDeviceContext()->Draw(vertexCount,0);
	}
	void D3DRendererAPI::SetLineWidth(float width) {
	}
}
