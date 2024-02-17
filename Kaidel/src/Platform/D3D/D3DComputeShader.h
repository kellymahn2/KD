#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
namespace Kaidel {

	
	class D3DUAVInput : public UAVInput {
	public:
		D3DUAVInput(uint32_t count,uint32_t sizeofElement, void* data = nullptr);
		~D3DUAVInput();
		virtual void SetBufferData(void* data, uint32_t count);
		virtual void Bind(uint32_t slot = 0)const;
		virtual void Unbind() const;

	private:
		ID3D11ShaderResourceView* m_SRV;
		ID3D11Buffer* m_Buffer;
		mutable uint32_t m_LastBoundSlot = 0;
		uint64_t m_Count;
		uint64_t m_SizeofElement;
	};
	class D3DTypedBufferInput : public TypedBufferInput {
	public:
		D3DTypedBufferInput(TypedBufferInputDataType type, TypedBufferAccessMode accessMode , uint32_t  width, uint32_t height, void* data = nullptr);
		~D3DTypedBufferInput();
		void SetBufferData(void* data, uint32_t width, uint32_t height);
		virtual uint64_t GetTextureID()const;
	private:
		void Bind(uint32_t slot)const override;
		void Unbind() const;
		ID3D11Buffer* m_Buffer;
		ID3D11ShaderResourceView* m_SRV;
		mutable uint32_t m_LastBoundSlot = 0;
		TypedBufferInputDataType m_InputType;
		TypedBufferAccessMode m_AccessMode;
		uint64_t m_Width, m_Height;
	};

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
