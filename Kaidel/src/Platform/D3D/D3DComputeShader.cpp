#include "KDpch.h"
#include "D3DComputeShader.h"
#include "D3DContext.h"
#include "D3Dcompiler.h"


namespace Kaidel {

	namespace Utils {
		inline std::string GetFileContents(const std::string& filepath) {
			std::ifstream file(filepath);

			std::string content((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());

			return content;
		}
		inline uint64_t GetTypedBufferInputDataSize(TypedBufferInputDataType type) {
			switch (type)
			{
			case Kaidel::TypedBufferInputDataType::None:
				return 0;
			case Kaidel::TypedBufferInputDataType::RGBA8:
				return 4;
			}
			return 0;
		}
		inline uint64_t GetTypedBufferAccessFlags(TypedBufferAccessMode accessMode) {
			switch (accessMode)
			{
			case Kaidel::Read: return D3D11_CPU_ACCESS_READ;
			case Kaidel::Write: return D3D11_CPU_ACCESS_WRITE;
			case Kaidel::ReadWrite: return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
			}
			return 0;
		}
	}

	D3DTypedBufferInput::D3DTypedBufferInput(TypedBufferInputDataType type, TypedBufferAccessMode accessMode  ,uint32_t width, uint32_t height, void* data)
		:m_InputType(type),m_AccessMode(accessMode),m_Width(width),m_Height(height)
	{
		auto d3dContext = D3DContext::Get();
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		uint64_t typeSize = Utils::GetTypedBufferInputDataSize(type);
		bd.ByteWidth = typeSize * width * height;
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bd.CPUAccessFlags = Utils::GetTypedBufferAccessFlags(accessMode);

		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = data;
		D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bd, &sd, &m_Buffer));

		D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(m_Buffer,nullptr,&m_SRV));
		
	}
	D3DTypedBufferInput::~D3DTypedBufferInput()
	{
		if (m_Buffer)
			m_Buffer->Release();
		if (m_SRV)
			m_SRV->Release();
	}
	void D3DTypedBufferInput::SetBufferData(void* data, uint32_t width, uint32_t height)
	{
		auto d3dContext = D3DContext::Get();

		if (m_Width!=width||m_Height != height) {
			m_Width = width;
			m_Height = height;
			D3D11_BUFFER_DESC bd{};
			bd.ByteWidth = width*height*Utils::GetTypedBufferInputDataSize(m_InputType);
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bd.CPUAccessFlags = Utils::GetTypedBufferAccessFlags(m_AccessMode);
			ID3D11Buffer* newVB = nullptr;
			D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bd, nullptr, &newVB));
			m_Buffer->Release();
			m_Buffer = newVB;
			if (m_SRV)
				m_SRV->Release();
			D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(m_Buffer, nullptr, &m_SRV));
		}
		D3D11_MAPPED_SUBRESOURCE mapped{};
		d3dContext->GetDeviceContext()->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, data, m_Width * m_Height * Utils::GetTypedBufferInputDataSize(m_InputType));
		d3dContext->GetDeviceContext()->Unmap(m_Buffer, 0);
	}
	uint64_t D3DTypedBufferInput::GetTextureID()const
	{
		return (uint64_t)m_SRV;
	}
	void D3DTypedBufferInput::Bind(uint32_t slot)const
	{
		auto d3dContext = D3DContext::Get();
		m_LastBoundSlot = slot;
		d3dContext->GetDeviceContext()->VSSetShaderResources(slot, 1, &m_SRV);
		d3dContext->GetDeviceContext()->PSSetShaderResources(slot, 1, &m_SRV);
		d3dContext->GetDeviceContext()->CSSetShaderResources(slot, 1, &m_SRV);
	}
	void D3DTypedBufferInput::Unbind() const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->VSSetShaderResources(m_LastBoundSlot, 1, &m_SRV);
		d3dContext->GetDeviceContext()->PSSetShaderResources(m_LastBoundSlot, 1, &m_SRV);
		d3dContext->GetDeviceContext()->CSSetShaderResources(m_LastBoundSlot, 1, &m_SRV);
	}

	D3DUAVInput::D3DUAVInput(uint32_t count,uint32_t sizeofElement,void* data)
		:m_SizeofElement(sizeofElement),m_Count(count?count:1)
	{
		auto d3dContext = D3DContext::Get();
		m_SizeofElement = sizeofElement;

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = sizeofElement;
		bufferDesc.ByteWidth = m_Count * sizeofElement;

		
		if (data == nullptr) {
			D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bufferDesc,nullptr, &m_Buffer));
		}
		else {
			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = data;
			D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_Buffer));
		}
		D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(m_Buffer, nullptr, &m_SRV));
		++UAVInput::s_UAVCount;
		m_Count = count;
	}
	D3DUAVInput::~D3DUAVInput() {
		if(m_Buffer)
			m_Buffer->Release();
		if (m_SRV)
			m_SRV->Release();
	}
	void D3DUAVInput::SetBufferData(void* data, uint32_t count) {
		auto d3dContext = D3DContext::Get();
		if (count > m_Count) {
			// If the new count exceeds the original count, resize the buffer
			m_Buffer->Release();
			m_SRV->Release();
			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			bufferDesc.StructureByteStride = m_SizeofElement;
			bufferDesc.ByteWidth = count * m_SizeofElement;

			D3D11_SUBRESOURCE_DATA initData = {};
			initData.pSysMem = data;

			D3DASSERT(d3dContext->GetDevice()->CreateBuffer(&bufferDesc, &initData, &m_Buffer));
			D3DASSERT(d3dContext->GetDevice()->CreateShaderResourceView(m_Buffer, nullptr, &m_SRV));
		}
		else {
			// If the new count does not exceed the original count, update the existing buffer
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			d3dContext->GetDeviceContext()->Map(m_Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, data, count * m_SizeofElement);
			d3dContext->GetDeviceContext()->Unmap(m_Buffer, 0);
		}
	}
	void D3DUAVInput::Bind(uint32_t slot)const {
		auto d3dContext = D3DContext::Get();
		m_LastBoundSlot = slot;
		d3dContext->GetDeviceContext()->PSSetShaderResources(slot, 1, &m_SRV);
		d3dContext->GetDeviceContext()->VSSetShaderResources(slot, 1, &m_SRV);
		d3dContext->GetDeviceContext()->CSSetShaderResources(slot, 1, &m_SRV);
	}
	void D3DUAVInput::Unbind() const {
		auto d3dContext = D3DContext::Get();
		ID3D11ShaderResourceView* srv = nullptr;
		d3dContext->GetDeviceContext()->VSSetShaderResources(m_LastBoundSlot, 1, &srv);
		d3dContext->GetDeviceContext()->PSSetShaderResources(m_LastBoundSlot, 1, &srv);
		d3dContext->GetDeviceContext()->CSSetShaderResources(m_LastBoundSlot, 1, &srv);
	}



	D3DComputeShader::D3DComputeShader(const std::string& filepath) {
		auto d3dContext = D3DContext::Get();
		ID3DBlob* csBlob = nullptr;
		{
			std::string src = Utils::GetFileContents(filepath);
			D3DASSERT(D3DCompile(src.c_str(), src.length(), nullptr, nullptr, nullptr, "main", "cs_5_0", 0, 0, &csBlob, nullptr));
		}
		D3DASSERT(d3dContext->GetDevice()->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(),nullptr, &m_ComputeShader));
		csBlob->Release();
	}
	D3DComputeShader::~D3DComputeShader() {
		if (m_ComputeShader != nullptr) {
			m_ComputeShader->Release();
		}
	}
	void D3DComputeShader::Bind()  const {
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->CSSetShader(m_ComputeShader, nullptr, 0);
	}
	void D3DComputeShader::Unbind()  const {
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->CSSetShader(nullptr, nullptr, 0);
	}
	void D3DComputeShader::Execute(uint64_t x, uint64_t y, uint64_t z)  const{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->Dispatch(x, y, z);
	}

}
