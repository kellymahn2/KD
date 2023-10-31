#pragma once

#include "Kaidel/Renderer/Framebuffer.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

namespace Kaidel {

	class D3DFrameBuffer : public Framebuffer
	{
	public:
		D3DFrameBuffer(const FramebufferSpecification& spec);
		virtual ~D3DFrameBuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const override {  return (uint64_t)m_ColorAttachments.at(index).SRV; }

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		struct FrameBufferTextureTarget {
			ID3D11Texture2D* Texture = nullptr;
			ID3D11RenderTargetView* RenderTargetView = nullptr;
			ID3D11Texture2D* StagingTexture = nullptr;
			ID3D11ShaderResourceView* SRV = nullptr;
		};
		struct FrameBufferDepthTarget {
			ID3D11Texture2D* Texture = nullptr;
			ID3D11DepthStencilView* DepthStencilView = nullptr;
			ID3D11DepthStencilState* DepthStencilState = nullptr;
		};



		void ClearAttachment(uint32_t attachmentIndex, const float* colors) override;

	private:

		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<FrameBufferTextureTarget> m_ColorAttachments;
		FrameBufferDepthTarget m_DepthAttachment{};
		bool cleared = false;
		ID3D11RasterizerState* m_RasterizerState = nullptr;
		D3D11_RASTERIZER_DESC m_Settings;
	};

}
