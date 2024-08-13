#pragma once
#include "Kaidel\Core\Buffer.h"
namespace Kaidel {

	enum class BufferStorageType{
		None = 0,
		Static,
		Dynamic
	};
	enum class ShaderDataType
	{
		None = 0,Dummy, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:    return 4;
			case ShaderDataType::Float2:   return 4 * 2;
			case ShaderDataType::Float3:   return 4 * 3;
			case ShaderDataType::Float4:   return 4 * 4;
			case ShaderDataType::Mat3:     return 4 * 3 * 3;
			case ShaderDataType::Mat4:     return 4 * 4 * 4;
			case ShaderDataType::Int:      return 4;
			case ShaderDataType::Int2:     return 4 * 2;
			case ShaderDataType::Int3:     return 4 * 3;
			case ShaderDataType::Int4:     return 4 * 4;
			case ShaderDataType::Bool:     return 1;
		}

		KD_CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;

		size_t Offset;
		uint32_t Size;
		uint32_t Divisor;
		bool Normalized;
		
		BufferElement() = default;

		//DummyElement
		BufferElement(uint32_t size, const std::string& name = "")
			:Name(name),Type(ShaderDataType::Dummy),Size(size),Offset(0),Divisor(0),Normalized(false)
		{
		}


		//Active Element
		BufferElement(ShaderDataType type, const std::string& name, uint32_t divisor = 0, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Divisor(divisor), Normalized(normalized)
		{
		}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:   return 1;
				case ShaderDataType::Float2:  return 2;
				case ShaderDataType::Float3:  return 3;
				case ShaderDataType::Float4:  return 4;
				case ShaderDataType::Mat3:    return 3; // 3* float3
				case ShaderDataType::Mat4:    return 4; // 4* float4
				case ShaderDataType::Int:     return 1;
				case ShaderDataType::Int2:    return 2;
				case ShaderDataType::Int3:    return 3;
				case ShaderDataType::Int4:    return 4;
				case ShaderDataType::Bool:    return 1;
			}

			KD_CORE_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(std::initializer_list<BufferElement> elements)
			: m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		void Push(std::initializer_list<BufferElement> elements)
		{
			for (auto& element : elements) {
				m_Elements.push_back(element);
			}
			CalculateOffsetsAndStride();
		}

		uint32_t GetStride() const { return m_Stride; }
		const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		void SetStride(uint32_t stride) { m_Stride = stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	// Currently Kaidel only supports 32-bit index buffers
	class IndexBuffer : public IRCCounter<false>
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};

	enum class VertexBufferMemoryType {
		None = 0,
		Static, //Make staging buffer each time on host, move to device memory at the end.
		Dynamic //Make and cache staging buffer on host, move to device memory at the end.
	};
	struct VertexBufferSpecification {
		uint32_t Size = 0;
		const void* Data = nullptr;
		VertexBufferMemoryType MemoryType = VertexBufferMemoryType::Dynamic;

	};
	class VertexBuffer : public IRCCounter<false>
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static Ref<VertexBuffer> Create(const VertexBufferSpecification& specification);
		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
	};
	

	class TransferBuffer : public IRCCounter<false> {
	public:
		virtual ~TransferBuffer() = default;

		virtual void* Map() = 0;
		virtual void Unmap() = 0;

		virtual void SetDataFromAllocation(const void* allocation, uint64_t size, uint64_t offset) = 0;
		virtual void GetData(void* out, uint64_t size, uint64_t offset) = 0;

		static Ref<TransferBuffer> Create(uint64_t size,const void* initData,uint64_t initDataSize = -1);
	};


}
