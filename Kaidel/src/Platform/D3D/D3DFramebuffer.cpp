#include "KDpch.h"
#include "Platform/D3D/D3DFrameBuffer.h"
#include "Platform/D3D/D3DContext.h"

namespace Kaidel {

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {





		static void CreateTextures(FramebufferSpecification& specs, bool multisampled, D3DFrameBuffer::FrameBufferTextureTarget* textures, uint32_t count)
		{
			auto d3dContext = D3DContext::Get();
			for (int i = 0; i < count; ++i) {
				D3D11_TEXTURE2D_DESC colorBufferDesc = {};
				colorBufferDesc.Width = specs.Width; // Set width of the texture
				colorBufferDesc.Height = specs.Height; // Set height of the texture
				colorBufferDesc.MipLevels = 1;
				colorBufferDesc.ArraySize = 1;
				colorBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				colorBufferDesc.SampleDesc.Count = 1;
				colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				colorBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
				D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&colorBufferDesc, nullptr, &(textures + i)->Texture));
			}
		}

		static bool IsDepthFormat(TextureFormat format)
		{
			switch (format)
			{
			case Kaidel::TextureFormat::Depth24Stencil8:
				return true;
			}

			return false;
		}

		static int KaidelFBTextureFormatToGL(TextureFormat format)
		{
			return 0;
		}

	}

	D3DFrameBuffer::D3DFrameBuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto spec : m_Specification.Attachments.Attachments) {
			if (!Utils::IsDepthFormat(spec.Format))
				m_ColorAttachmentSpecifications.emplace_back(spec);
			else
				m_DepthAttachmentSpecification = spec;

		}
		Invalidate();
	}

	D3DFrameBuffer::~D3DFrameBuffer()
	{
		if (m_RendererID) {
			for (auto& colorAttachment : m_ColorAttachments) {
				if (colorAttachment.Texture)
					colorAttachment.Texture->Release();
				if (colorAttachment.RenderTargetView)
					colorAttachment.RenderTargetView->Release();
				if (colorAttachment.StagingTexture)
					colorAttachment.StagingTexture->Release();
				if (colorAttachment.SRV)
					colorAttachment.SRV->Release();
			}
			m_ColorAttachments.clear();
			if (m_DepthAttachment.DepthStencilView) {
				m_DepthAttachment.DepthStencilView->Release();
			}
			if (m_DepthAttachment.Texture)
				m_DepthAttachment.Texture->Release();
		}
	}

	void D3DFrameBuffer::Invalidate()
	{
		if (m_RendererID) {
			for (auto& colorAttachment : m_ColorAttachments) {
				if (colorAttachment.Texture)
					colorAttachment.Texture->Release();
				if (colorAttachment.RenderTargetView)
					colorAttachment.RenderTargetView->Release();
				if (colorAttachment.SRV)
					colorAttachment.SRV->Release();
				if (colorAttachment.StagingTexture)
					colorAttachment.StagingTexture->Release();
			}
			m_ColorAttachments.clear();
			if (m_DepthAttachment.DepthStencilView) {
				m_DepthAttachment.DepthStencilView->Release();
			}
			if (m_DepthAttachment.Texture)
				m_DepthAttachment.Texture->Release();
			if (m_DepthAttachment.DepthStencilState)
				m_DepthAttachment.DepthStencilState->Release();
		}
		bool multisample = m_Specification.Samples > 1;
		auto d3dContext = D3DContext::Get();
		if (m_ColorAttachmentSpecifications.size()) {
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			auto current = m_ColorAttachments.data();
			for (auto& colorspec : m_ColorAttachmentSpecifications) {
				D3D11_TEXTURE2D_DESC colorBufferDesc = {};
				colorBufferDesc.Width = m_Specification.Width;
				colorBufferDesc.Height = m_Specification.Height;
				colorBufferDesc.MipLevels = 1;
				colorBufferDesc.ArraySize = 1;
				colorBufferDesc.SampleDesc.Count = 1;
				colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				colorBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				colorBufferDesc.CPUAccessFlags = 0;
				switch (colorspec.Format)
				{
				case TextureFormat::RGBA8:
					colorBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				case TextureFormat::R32I:
					colorBufferDesc.Format = DXGI_FORMAT_R32_SINT;
					break;
				}
				//TODO: Add readable functionality.
				D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&colorBufferDesc, nullptr, &current->Texture));
				if (colorspec.Readable) {
					colorBufferDesc.Usage = D3D11_USAGE_STAGING;
					colorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
					colorBufferDesc.BindFlags = 0;
					D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&colorBufferDesc, nullptr, &current->StagingTexture));
				}
				D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(current->Texture, nullptr, &current->SRV));
				D3DASSERT(d3dContext->GetDevice()->CreateRenderTargetView(current->Texture, nullptr, &current->RenderTargetView));
				++current;
			}
		}
		if (m_DepthAttachmentSpecification.Format != TextureFormat::None) {


			/*D3D11_TEXTURE2D_DESC depthStencilDesc = {};
			depthStencilDesc.Width = m_Specification.Width;
			depthStencilDesc.Height = m_Specification.Height;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
			depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;


			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
			depthStencilViewDesc.Format = depthStencilDesc.Format;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthAttachment.Texture));
			D3DASSERT(d3dContext->GetDevice()->CreateDepthStencilView(m_DepthAttachment.Texture, &depthStencilViewDesc, &m_DepthAttachment.DepthStencilView));
			D3D11_DEPTH_STENCIL_DESC dsd{};
			dsd.DepthEnable = TRUE;
			dsd.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
			dsd.DepthFunc	= D3D11_COMPARISON_LESS;
			dsd.StencilEnable	= false;
			dsd.StencilReadMask	= D3D11_DEFAULT_STENCIL_READ_MASK;
			dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
			dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			D3DASSERT(d3dContext->GetDevice()->CreateDepthStencilState(&dsd, &m_DepthAttachment.DepthStencilState));*/

			// create depth stensil state
			D3D11_DEPTH_STENCIL_DESC dsDesc = {};
			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
			D3DASSERT(d3dContext->GetDevice()->CreateDepthStencilState(&dsDesc, &m_DepthAttachment.DepthStencilState));

			// bind depth state
			d3dContext->GetDeviceContext()->OMSetDepthStencilState(m_DepthAttachment.DepthStencilState, 1u);

			// create depth stensil texture
			D3D11_TEXTURE2D_DESC descDepth = {};
			descDepth.Width = m_Specification.Width;
			descDepth.Height = m_Specification.Height;
			descDepth.MipLevels = 1u;
			descDepth.ArraySize = 1u;
			descDepth.Format = DXGI_FORMAT_D32_FLOAT;
			descDepth.SampleDesc.Count = 1u;
			descDepth.SampleDesc.Quality = 0u;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&descDepth, nullptr, &m_DepthAttachment.Texture));

			// create view of depth stensil texture
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
			descDSV.Format = DXGI_FORMAT_D32_FLOAT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0u;
			D3DASSERT(d3dContext->GetDevice()->CreateDepthStencilView(
				m_DepthAttachment.Texture, &descDSV, &m_DepthAttachment.DepthStencilView
			));


		}
		m_Settings.CullMode = D3D11_CULL_NONE;
		m_Settings.FillMode = D3D11_FILL_SOLID;
		m_Settings.FrontCounterClockwise = true;
		m_Settings.DepthBias = 0;
		m_Settings.SlopeScaledDepthBias = 0.0f;
		m_Settings.DepthBiasClamp = 0.0f;
		m_Settings.DepthClipEnable = TRUE;
		m_Settings.ScissorEnable = FALSE;
		m_Settings.MultisampleEnable = FALSE;
		m_Settings.AntialiasedLineEnable = FALSE;
		D3DASSERT(d3dContext->GetDevice()->CreateRasterizerState(&m_Settings, &m_RasterizerState));







		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MaxDepth = 1.0f;
		viewport.Width = m_Specification.Width;
		viewport.Height = m_Specification.Height;
		d3dContext->GetDeviceContext()->RSSetViewports(1, &viewport);
		m_RendererID = 1;
	}

	void D3DFrameBuffer::Bind()
	{
		auto d3dContext = D3DContext::Get();
		std::vector<ID3D11RenderTargetView*> rtvs(m_ColorAttachments.size(), nullptr);

		for (size_t i = 0; i < m_ColorAttachments.size(); ++i)
			rtvs.at(i) = m_ColorAttachments.at(i).RenderTargetView;
		d3dContext->GetDeviceContext()->RSSetState(m_RasterizerState);
		d3dContext->GetDeviceContext()->OMSetRenderTargets(rtvs.size(), rtvs.data(), m_DepthAttachment.DepthStencilView);
		d3dContext->GetDeviceContext()->OMSetDepthStencilState(m_DepthAttachment.DepthStencilState, 1);
		cleared = false;
	}

	void D3DFrameBuffer::Unbind()
	{
		auto d3dContext = D3DContext::Get();
		auto backBuffer = d3dContext->GetBackBuffer();
		d3dContext->GetDeviceContext()->OMSetRenderTargets(1, &backBuffer, d3dContext->GetDepthStencilView());
		d3dContext->GetDeviceContext()->OMSetDepthStencilState(nullptr, 0);
	}

	void D3DFrameBuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width * height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize) {
			KD_CORE_WARN("Attempted to resize framebuffer to {} , {}", width, height);
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	

	void D3DFrameBuffer::ClearAttachment(uint32_t attachmentIndex, const void* colors)
	{
		/*auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->ClearRenderTargetView(m_ColorAttachments.at(attachmentIndex).RenderTargetView, colors);
		if (!cleared) {
			d3dContext->GetDeviceContext()->ClearDepthStencilView(m_DepthAttachment.DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
			cleared = true;
		}*/
	}


}
