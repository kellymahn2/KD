#include "KDpch.h"
#include "Platform/D3D/D3DRendererAPI.h"
#include "Platform/D3D/D3DContext.h"

namespace Kaidel {
	

	void D3DRendererAPI::Init()
	{

		m_RasterizerDescriptor.FillMode = D3D11_FILL_SOLID;
		m_RasterizerDescriptor.CullMode = D3D11_CULL_NONE;
		m_RasterizerDescriptor.FrontCounterClockwise = TRUE;
		m_RasterizerDescriptor.DepthBias = 0.0f;
		m_RasterizerDescriptor.SlopeScaledDepthBias = 0.0f;
		m_RasterizerDescriptor.DepthBiasClamp = 0.0f;
		m_RasterizerDescriptor.DepthClipEnable = TRUE;
		m_RasterizerDescriptor.ScissorEnable = FALSE;
		m_RasterizerDescriptor.MultisampleEnable= FALSE;
		m_RasterizerDescriptor.AntialiasedLineEnable = FALSE;
		auto d3dContext = D3DContext::Get();

		ID3D11RasterizerState* rasterizerState = nullptr;

		D3DASSERT(d3dContext->GetDevice()->CreateRasterizerState(&m_RasterizerDescriptor, &rasterizerState));
		d3dContext->GetDeviceContext()->RSSetState(rasterizerState);
		rasterizerState->Release();
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
		viewport.MaxDepth = 1.0f;
		d3dContext->GetDeviceContext()->RSSetViewports(1, &viewport);
	}

	void D3DRendererAPI::GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) {
		auto d3dContext = D3DContext::Get();
		D3D11_VIEWPORT viewport;
		UINT num = 1;
		d3dContext->GetDeviceContext()->RSGetViewports(&num, &viewport);
		y = viewport.TopLeftX;
		y = viewport.TopLeftY;
		width = viewport.Width;
		height = viewport.Height;
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
		if (m_CullMode == CullMode::FrontAndBack)
			return;
		vertexArray->Bind();
		auto d3dContext = D3DContext::Get();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		d3dContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3dContext->GetDeviceContext()->DrawIndexed(count, 0, 0);
	}

	void D3DRendererAPI::DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) {
		if (m_CullMode == CullMode::FrontAndBack)
			return;
		vertexArray->Bind();
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		d3dContext->GetDeviceContext()->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
	}

	void D3DRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		if (m_CullMode == CullMode::FrontAndBack)
			return; 
		vertexArray->Bind();
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		d3dContext->GetDeviceContext()->Draw(vertexCount,0);
	}

	void D3DRendererAPI::DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
		if (m_CullMode == CullMode::FrontAndBack)
			return;
		vertexArray->Bind();
		auto d3dContext = D3DContext::Get();
		D3D11_PRIMITIVE_TOPOLOGY topology = (D3D11_PRIMITIVE_TOPOLOGY)((uint32_t)(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST)+m_PathVertexCount);
		d3dContext->GetDeviceContext()->IASetPrimitiveTopology(topology);
		d3dContext->GetDeviceContext()->Draw(vertexCount, 0);
	}

	void D3DRendererAPI::DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
		if (m_CullMode == CullMode::FrontAndBack)
			return;
		vertexArray->Bind();
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		d3dContext->GetDeviceContext()->Draw(vertexCount, 0);
	}

	void D3DRendererAPI::SetLineWidth(float width) {

	}
	void D3DRendererAPI::SetPointSize(float pixelSize) {
	}

	void D3DRendererAPI::SetCullMode(CullMode cullMode) {
		
		m_CullMode = cullMode;
		switch (cullMode)
		{
		case Kaidel::CullMode::None:m_RasterizerDescriptor.CullMode = D3D11_CULL_NONE; break;
		case Kaidel::CullMode::Front:m_RasterizerDescriptor.CullMode = D3D11_CULL_FRONT; break;
		case Kaidel::CullMode::Back:m_RasterizerDescriptor.CullMode = D3D11_CULL_BACK; break;
		case Kaidel::CullMode::FrontAndBack:return;
		}

		auto d3dContext = D3DContext::Get();
		
		ID3D11RasterizerState* rasterizerState = nullptr;

		D3DASSERT(d3dContext->GetDevice()->CreateRasterizerState(&m_RasterizerDescriptor, &rasterizerState));
		d3dContext->GetDeviceContext()->RSSetState(rasterizerState);
		rasterizerState->Release();
	}

	int D3DRendererAPI::QueryMaxTextureSlots() {

		return D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
	}

	float D3DRendererAPI::QueryMaxTessellationLevel() {
		return D3D11_TESSELLATOR_MAX_TESSELLATION_FACTOR;
	}
	void D3DRendererAPI::SetPatchVertexCount(uint32_t count) {
		m_PathVertexCount = count;
	}
	void D3DRendererAPI::SetDefaultTessellationLevels(const glm::vec4& outer, const glm::vec2& inner) {

	}
}
