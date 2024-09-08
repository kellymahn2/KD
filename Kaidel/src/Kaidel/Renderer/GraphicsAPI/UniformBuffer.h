#pragma once
#include "Kaidel/Core/Base.h"
#include "ShaderInputBuffer.h"
namespace Kaidel {
	class UniformBuffer : public ShaderInputBuffer {
	public:
		virtual ~UniformBuffer() = default;
		virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;
		static Ref<UniformBuffer> Create(uint64_t size);
	};
}
