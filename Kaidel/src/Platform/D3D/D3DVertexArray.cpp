#include "KDpch.h"
#include "Platform/D3D/D3DVertexArray.h"
#include "Platform/D3D/D3DContext.h"
#include "Platform/D3D/D3DShader.h"
#include "Platform/D3D/D3DBuffer.h"
#include <glad/glad.h>

namespace Kaidel {
	namespace Utils {
		static const char* ShaderDataTypeToD3DSemanticName(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Float:return "TEXCOORD";
			case ShaderDataType::Float2:return "TEXCOORD";
			case ShaderDataType::Float3:return "POSITION";
			case ShaderDataType::Float4:return "COLOR";
			case ShaderDataType::Int:return "TEXCOORD";
			case ShaderDataType::Int2:return "TEXCOORD";
			case ShaderDataType::Int3:return "TEXCOORD";
			case ShaderDataType::Int4:return "TEXCOORD";
			case ShaderDataType::Bool:return "TEXCOORD";
			}
			KD_CORE_ASSERT(false, "Not implemented");
			return 0;
		}
		static DXGI_FORMAT ShaderDataTypeToD3DFormat(ShaderDataType type) {
			switch (type)
			{
			case ShaderDataType::Float:return DXGI_FORMAT_R32_FLOAT;
			case ShaderDataType::Float2:return DXGI_FORMAT_R32G32_FLOAT;
			case ShaderDataType::Float3:return DXGI_FORMAT_R32G32B32_FLOAT;
			case ShaderDataType::Float4:return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case ShaderDataType::Mat3:return DXGI_FORMAT_R32G32B32_FLOAT;
			case ShaderDataType::Mat4:return DXGI_FORMAT_R32G32B32A32_FLOAT;
			case ShaderDataType::Int:return DXGI_FORMAT_R32_SINT;
			case ShaderDataType::Int2:return DXGI_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:return DXGI_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:return DXGI_FORMAT_R32G32B32A32_SINT;
			case ShaderDataType::Bool:return DXGI_FORMAT_R32_SINT;
			}
			KD_CORE_ASSERT(false, "Not implemented");
			return DXGI_FORMAT_A8_UNORM;
		}


		static void AddVertexBuffer(Ref<VertexBuffer> vertexBuffer, std::vector<D3D11_INPUT_ELEMENT_DESC>& descriptors, std::unordered_map<std::string, uint32_t>& semantics) {

			const BufferLayout& layout = vertexBuffer->GetLayout();
			for (const BufferElement& element : layout) {

				if (element.Type == ShaderDataType::Mat3 || element.Type == ShaderDataType::Mat4) {
					uint32_t count = element.GetComponentCount();
					for (uint32_t i = 0; i < count; ++i) {
						D3D11_INPUT_ELEMENT_DESC descriptor{};
						uint32_t offset = (element.Offset + sizeof(float) * count * i);
						descriptor.AlignedByteOffset = offset;
						descriptor.Format = ShaderDataTypeToD3DFormat(element.Type);
						descriptor.InputSlot = descriptors.size();
						descriptor.InputSlotClass = element.Divisor ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
						descriptor.InstanceDataStepRate = element.Divisor;
						descriptor.SemanticName = "MATRIX";
						descriptor.SemanticIndex = semantics["MATRIX"]++;
						descriptors.push_back(descriptor);
					}
					continue;
				}

				D3D11_INPUT_ELEMENT_DESC descriptor{};
				descriptor.AlignedByteOffset = element.Offset;
				descriptor.Format = ShaderDataTypeToD3DFormat(element.Type);
				descriptor.InputSlot = descriptors.size();
				descriptor.InputSlotClass = element.Divisor ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
				descriptor.InstanceDataStepRate = element.Divisor;

				std::string semanticName = ShaderDataTypeToD3DSemanticName(element.Type);
				descriptor.SemanticName = semanticName.c_str();
				uint32_t semanticIndex = semantics[semanticName]++;
				descriptor.SemanticIndex = semanticIndex;

				descriptors.push_back(descriptor);
			}
		}

		static ID3D11InputLayout* CreateVertexArray(const VertexArraySpecification& spec) {


			std::vector<D3D11_INPUT_ELEMENT_DESC> descriptors;
			std::unordered_map<std::string, uint32_t> semantics;
			for (auto& vertexBuffer : spec.VertexBuffers) {
				AddVertexBuffer(vertexBuffer, descriptors,semantics);
			}
			auto d3dContext = D3DContext::Get();

			D3DShader* d3dShader = (D3DShader*)spec.UsedShader.get();
			ID3DBlob* blob = d3dShader->GetVSBlob();

			ID3D11InputLayout* inputLayout = nullptr;
			D3DASSERT(d3dContext->GetDevice()->CreateInputLayout(descriptors.data(),descriptors.size(),blob->GetBufferPointer(),blob->GetBufferSize(),&inputLayout));

			return inputLayout;
		}
	}
	D3DVertexArray::D3DVertexArray(const VertexArraySpecification& spec)
		:m_Specification(spec)
	{
		m_InputLayout = Utils::CreateVertexArray(spec);
	}

	D3DVertexArray::~D3DVertexArray()
	{
		if (m_InputLayout)
			m_InputLayout->Release();
	}

	void D3DVertexArray::Bind() const
	{
		auto d3dContext = D3DContext::Get();
	
		d3dContext->GetDeviceContext()->IASetInputLayout(m_InputLayout);
		if (m_Specification.IndexBuffer)
			m_Specification.IndexBuffer->Bind();
		uint32_t i = 0;
		for (auto& vbo : m_Specification.VertexBuffers) {
			D3DVertexBuffer* buffer = (D3DVertexBuffer*)vbo.get();

			UINT stride = buffer->m_Layout.GetStride();
			UINT offset = 0;
			d3dContext->GetDeviceContext()->IASetVertexBuffers(i, 1, &buffer->m_VertexBuffer, &stride, &offset);
			++i;
		}
	}

	void D3DVertexArray::Unbind() const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetInputLayout(nullptr);
	}

	D3DVertexArray* D3DVertexArray::GetCurrentBound()
	{
		return nullptr;
	}

}
