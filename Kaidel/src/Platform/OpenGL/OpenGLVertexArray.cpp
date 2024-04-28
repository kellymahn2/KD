#include "KDpch.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Kaidel {
	namespace Utils {
		static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
		{
			switch (type)
			{
				case ShaderDataType::Float:    return GL_FLOAT;
				case ShaderDataType::Float2:   return GL_FLOAT;
				case ShaderDataType::Float3:   return GL_FLOAT;
				case ShaderDataType::Float4:   return GL_FLOAT;
				case ShaderDataType::Mat3:     return GL_FLOAT;
				case ShaderDataType::Mat4:     return GL_FLOAT;
				case ShaderDataType::Int:      return GL_INT;
				case ShaderDataType::Int2:     return GL_INT;
				case ShaderDataType::Int3:     return GL_INT;
				case ShaderDataType::Int4:     return GL_INT;
				case ShaderDataType::Bool:     return GL_BOOL;
			}
			KD_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}

		static void AddVertexBuffer(uint32_t vertexArray,uint32_t& currentVertexBufferIndex,Ref<VertexBuffer> vertexBuffer) {
			KD_CORE_ASSERT(vertexBuffer->GetLayout().GetElements().size(), "Vertex Buffer has no layout!");

			glBindVertexArray(vertexArray);
			vertexBuffer->Bind();

			const auto& layout = vertexBuffer->GetLayout();
			for (const auto& element : layout)
			{
				switch (element.Type)
				{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				{
					glEnableVertexAttribArray(currentVertexBufferIndex);
					glVertexAttribPointer(currentVertexBufferIndex,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)element.Offset);
					glVertexAttribDivisor(currentVertexBufferIndex, element.Divisor);
					currentVertexBufferIndex++;
					break;
				}
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(currentVertexBufferIndex);
					glVertexAttribIPointer(currentVertexBufferIndex,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.Type),
						layout.GetStride(),
						(const void*)element.Offset);
					glVertexAttribDivisor(currentVertexBufferIndex, element.Divisor);
					currentVertexBufferIndex++;
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					uint8_t count = element.GetComponentCount();
					for (uint8_t i = 0; i < count; i++)
					{
						glEnableVertexAttribArray(currentVertexBufferIndex);
						uint32_t offset = (uint32_t)(element.Offset + sizeof(float) * count * i);
						glVertexAttribPointer(currentVertexBufferIndex,
							count,
							ShaderDataTypeToOpenGLBaseType(element.Type),
							element.Normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(uint64_t)offset);
						glVertexAttribDivisor(currentVertexBufferIndex, element.Divisor);
						currentVertexBufferIndex++;
					}
					break;
				}
				default:
					KD_CORE_ASSERT(false, "Unknown ShaderDataType!");
				}
			}	
		}

		static uint32_t CreateVertexArray(const VertexArraySpecification& spec,uint32_t& currentVertexBufferIndex) {
			uint32_t vertexArray;
			glCreateVertexArrays(1, &vertexArray);
			glBindVertexArray(vertexArray);
			for (auto& vb : spec.VertexBuffers) {
				AddVertexBuffer(vertexArray, currentVertexBufferIndex, vb);
			}
			if(spec.IndexBuffer)
				spec.IndexBuffer->Bind();
			glBindVertexArray(0);
			return vertexArray;
		}
	}

	void OpenGLVertexArray::SetIndexBuffer(Ref<IndexBuffer> indexBuffer) {
		m_Specification.IndexBuffer = indexBuffer;
		Bind();
		indexBuffer->Bind();
		Unbind();
	}

	void OpenGLVertexArray::SetVertexBuffers(std::initializer_list<Ref<VertexBuffer>> buffers)
	{
		glBindVertexArray(m_RendererID);
		m_Specification.VertexBuffers = buffers;
		uint32_t currentVertexBufferIndex = 0;
		for (auto& vb : m_Specification.VertexBuffers) {
			Utils::AddVertexBuffer(m_RendererID, currentVertexBufferIndex, vb);
		}

		for (uint32_t i = currentVertexBufferIndex; i < m_CurrentVertexBufferIndex;++i) {
			glDisableVertexAttribArray(i);
		}
		glBindVertexArray(0);
	}


	OpenGLVertexArray::OpenGLVertexArray(const VertexArraySpecification& spec)
		:m_Specification(spec)
	{
		m_RendererID = Utils::CreateVertexArray(spec,m_CurrentVertexBufferIndex);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{

		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}
}
