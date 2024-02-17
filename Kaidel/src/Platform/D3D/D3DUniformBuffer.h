#pragma once
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
namespace Kaidel {

	class D3DUniformBuffer:public UniformBuffer
	{
	public:
		D3DUniformBuffer(uint32_t size, uint32_t binding);
		virtual ~D3DUniformBuffer();

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

		void Bind(uint32_t binding) override;


		void Bind() override;


		void UnBind() override;

	private:
		uint32_t m_RendererID = 0;
		ID3D11Buffer* m_UniformBuffer;
	};
}

