#include "KDpch.h"
#include "Platform/D3D/D3DVertexArray.h"
#include "Platform/D3D/D3DContext.h"
#include "Platform/D3D/D3DShader.h"
#include <glad/glad.h>

namespace Kaidel {
	static const char* ShaderDataTypeToD3DBaseType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:return "TEXCOORD";
		case ShaderDataType::Float2:return "TEXCOORD";
		case ShaderDataType::Float3:return "POSITION";
		case ShaderDataType::Float4:return "COLOR";
		case ShaderDataType::Mat3:break;
		case ShaderDataType::Mat4:break;
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
		case ShaderDataType::Mat3:break;
		case ShaderDataType::Mat4:break;
		case ShaderDataType::Int:return DXGI_FORMAT_R32_SINT;
		case ShaderDataType::Int2:return DXGI_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3:return DXGI_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4:return DXGI_FORMAT_R32G32B32A32_SINT;
		case ShaderDataType::Bool:return DXGI_FORMAT_R32_SINT;
		}
		KD_CORE_ASSERT(false, "Not implemented");
		return DXGI_FORMAT_A8_UNORM;
	}
	D3DVertexArray::D3DVertexArray()
	{

	}

	D3DVertexArray::D3DVertexArray(Ref<Shader> shader)
		:m_Shader(shader)
	{

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
	}

	void D3DVertexArray::Unbind() const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->IASetInputLayout(m_InputLayout);
	}

	void D3DVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		m_VertexBuffers.push_back(vertexBuffer);
		const auto& layout = vertexBuffer->GetLayout();
		std::vector<D3D11_INPUT_ELEMENT_DESC> descs;
		descs.resize(0);
		std::unordered_map<std::string_view, UINT> indexMap;
		uint32_t currOffset = 0;
		for (const auto& element : layout) {
			auto semanticType = ShaderDataTypeToD3DBaseType(element.Type);

			switch (element.Type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			{
				descs.push_back(
					{ semanticType,indexMap[semanticType],ShaderDataTypeToD3DFormat(element.Type),
					0
					,currOffset,D3D11_INPUT_PER_VERTEX_DATA,0 });
				currOffset += element.Size;
				break;
			}
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Bool:
			{
				descs.push_back(
					{
						semanticType,indexMap[semanticType],ShaderDataTypeToD3DFormat(element.Type),
						0,
						currOffset,D3D11_INPUT_PER_VERTEX_DATA,0
					}
				);
				currOffset += element.Size;
				break;
			}
			default:
				KD_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}
			++indexMap[semanticType];
		}
		auto d3dContext = D3DContext::Get();
		auto shader = std::dynamic_pointer_cast<D3DShader>(m_Shader);
		D3DASSERT(d3dContext->GetDevice()->CreateInputLayout(descs.data(), descs.size(),
			shader->GetVSBlob()->GetBufferPointer(),shader->GetVSBlob()->GetBufferSize(),&m_InputLayout));

	}

	void D3DVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		m_IndexBuffer = indexBuffer;
	}

	D3DVertexArray* D3DVertexArray::GetCurrentBound()
	{
		return nullptr;
	}

}
