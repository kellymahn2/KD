#pragma once
#include "Kaidel/Renderer/Shader.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
namespace Kaidel {

	
	class D3DComputeShader : public ComputeShader {

	public:
		D3DComputeShader(const std::string& filepath);
		~D3DComputeShader();
		virtual void Bind()  const override;
		virtual void Unbind()  const override ;
		virtual void Execute(uint64_t x, uint64_t y, uint64_t z)  const  override;
	private:
		ID3D11ComputeShader* m_ComputeShader = nullptr;

	};
}
