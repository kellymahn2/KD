#include "KDpch.h"
#include "Platform/D3D/D3DFrameBuffer.h"
#include "Platform/D3D/D3DContext.h"
#include "Kaidel\Core\Timer.h"
namespace Kaidel {

	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {

		static int TextureTarget(bool multisampled)
		{
			return 0;
		}

		static void CreateTextures(FramebufferSpecification& specs,bool multisampled, D3DFrameBuffer::FrameBufferTextureTarget* textures, uint32_t count)
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

		static void BindTexture(bool multisampled, uint32_t id)
		{
		}

		static void AttachColorTexture(uint32_t id, int samples, int internalFormat, int format, uint32_t width, uint32_t height, int index)
		{
			
		}

		static void AttachDepthTexture(uint32_t id, int samples, int format, int attachmentType, uint32_t width, uint32_t height)
		{
			
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case Kaidel::FramebufferTextureFormat::DEPTH24STENCIL8:
				return true;
			}

			return false;
		}

		static int KaidelFBTextureFormatToGL(FramebufferTextureFormat format)
		{
			return 0;
		}

	}

	D3DFrameBuffer::D3DFrameBuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto spec : m_Specification.Attachments.Attachments) {
			if (!Utils::IsDepthFormat(spec.TextureFormat))
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
		}
		bool multisample = m_Specification.Samples>1;
		auto d3dContext = D3DContext::Get();
		if (m_ColorAttachmentSpecifications.size()) {
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			auto current = m_ColorAttachments.data();
			for (auto& colorspec : m_ColorAttachmentSpecifications) {
				D3D11_TEXTURE2D_DESC colorBufferDesc = {};
				colorBufferDesc.Width = m_Specification.Width;
				colorBufferDesc.Height =m_Specification.Height;
				colorBufferDesc.MipLevels = 1;
				colorBufferDesc.ArraySize = 1;
				colorBufferDesc.SampleDesc.Count = 1;
				colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				colorBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				colorBufferDesc.CPUAccessFlags = 0;
				switch (colorspec.TextureFormat)
				{
				case FramebufferTextureFormat::RGBA8:
					colorBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;
				case FramebufferTextureFormat::RED_INTEGER:
					colorBufferDesc.Format = DXGI_FORMAT_R32_SINT;
					break;
				}
				//TODO: Add readable functionality.
				D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&colorBufferDesc, nullptr, &current->Texture));
				if (colorspec.Readable) {
					colorBufferDesc.Usage = D3D11_USAGE_STAGING;
					colorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
					colorBufferDesc.BindFlags = 0;
					colorBufferDesc.Width = 1;
					colorBufferDesc.Height = 1;
					D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&colorBufferDesc, nullptr, &current->StagingTexture));
				}
				D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(current->Texture,nullptr, &current->SRV));
				D3DASSERT(d3dContext->GetDevice()->CreateRenderTargetView(current->Texture, nullptr, &current->RenderTargetView));
				++current;
			}
		}
		if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None) {
			D3D11_TEXTURE2D_DESC depthStencilDesc = {};
			depthStencilDesc.Width =  m_Specification.Width; 
			depthStencilDesc.Height = m_Specification.Height; 
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthStencilDesc.SampleDesc.Count = 1;
			depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
			depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthAttachment.Texture));
			D3DASSERT(d3dContext->GetDevice()->CreateDepthStencilView(m_DepthAttachment.Texture, nullptr, &m_DepthAttachment.DepthStencilView));
		}
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = m_Specification.Width;
		viewport.Height = m_Specification.Height;
		d3dContext->GetDeviceContext()->RSSetViewports(1, &viewport);
		m_RendererID = 1;
	}

	void D3DFrameBuffer::Bind()
	{
		auto d3dContext = D3DContext::Get();
		std::vector<ID3D11RenderTargetView*> rtvs(m_ColorAttachments.size(),nullptr);
		for (size_t i = 0; i < m_ColorAttachments.size();++i)
			rtvs.at(i) = m_ColorAttachments.at(i).RenderTargetView;
		d3dContext->GetDeviceContext()->OMSetRenderTargets(rtvs.size(), rtvs.data(), m_DepthAttachment.DepthStencilView);
	}

	void D3DFrameBuffer::Unbind()
	{
		auto d3dContext = D3DContext::Get();
		auto backBuffer = d3dContext->GetBackBuffer();
		/*ID3D11CommandList* commandList=nullptr;
		D3DASSERT(d3dContext->GetDefferedDeviceContext()->FinishCommandList(false, &commandList));
		d3dContext->GetDeviceContext()->ExecuteCommandList(commandList, false);
		if (commandList)
			commandList->Release();*/
		d3dContext->GetDeviceContext()->OMSetRenderTargets(1, &backBuffer, d3dContext->GetDepthStencilView());
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

	int D3DFrameBuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		KD_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());
		KD_CORE_ASSERT(m_ColorAttachmentSpecifications.at(attachmentIndex).Readable);
		auto d3dContext = D3DContext::Get();
		ID3D11Texture2D* texture;
		D3D11_TEXTURE2D_DESC td{}; {
			td.Width = 1;
			td.Height = 1;
			td.MipLevels = 1;
			td.ArraySize = 1;
			td.SampleDesc.Count = 1;
			td.Usage = D3D11_USAGE_STAGING;
			td.BindFlags =0;
			td.Format = DXGI_FORMAT_R32_SINT;
			td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			D3DASSERT(d3dContext->GetDevice()->CreateTexture2D(&td, nullptr, &texture));
		}
		D3D11_BOX box{};
		{
			box.left = x;
			box.top = y;
			box.right = x + 1;
			box.bottom = y + 1;
			box.front = 0;
			box.back = 1;
			d3dContext->GetDeviceContext()->CopySubresourceRegion(texture,
				0, 0, 0, 0, m_ColorAttachments.at(attachmentIndex).Texture, 0, &box);
		}
		// Map the texture
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		{
			D3DASSERT(d3dContext->GetDeviceContext()->Map(texture, 0, D3D11_MAP_READ, 0, &mappedResource));
		}
		// Access the pixel data
		UINT width = m_Specification.Width;  		
		UINT height = m_Specification.Height;  
		char* pixel = static_cast<char*>(mappedResource.pData)/*+ y * mappedResource.RowPitch+ x * 4*/;
		auto ret = *(int*)pixel;

		// Unmap the texture
		d3dContext->GetDeviceContext()->Unmap(texture, 0);
		if (texture)
			texture->Release();
		return ret;
	}

	void D3DFrameBuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		auto d3dContext = D3DContext::Get();
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		D3DASSERT(d3dContext->GetDeviceContext()->Map(m_ColorAttachments[attachmentIndex].StagingTexture, 0, D3D11_MAP_WRITE, 0, &mappedResource));
		int* colorValue = (int*)mappedResource.pData;
		uint64_t numPixels = m_Specification.Width * m_Specification.Height;
		for (uint64_t i = 0; i < numPixels; ++i)
			colorValue[i] = value;
		d3dContext->GetDeviceContext()->Unmap(m_ColorAttachments[attachmentIndex].StagingTexture, 0);
		d3dContext->GetDeviceContext()->CopyResource(m_ColorAttachments[attachmentIndex].Texture,m_ColorAttachments[attachmentIndex].StagingTexture);
	}

	void D3DFrameBuffer::ClearAttachment(uint32_t attachmentIndex, const float* colors)
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->ClearRenderTargetView(m_ColorAttachments.at(attachmentIndex).RenderTargetView, colors);
		d3dContext->GetDeviceContext()->ClearDepthStencilView(m_DepthAttachment.DepthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
	}


}
