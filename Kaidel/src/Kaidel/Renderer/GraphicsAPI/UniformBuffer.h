#pragma once
#include "Kaidel/Core/Base.h"
#include "Buffer.h"
namespace Kaidel {
	class UniformBuffer : public Buffer {
	public:
		virtual ~UniformBuffer() = default;
		virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;
		virtual BufferType GetBufferType()const { return BufferType::UniformBuffer; }



		static Ref<UniformBuffer> Create(uint64_t size);
	};
}
