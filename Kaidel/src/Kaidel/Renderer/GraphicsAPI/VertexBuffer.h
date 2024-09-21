#pragma once
#include "Kaidel/Core/Base.h"
#include "Buffer.h"

namespace Kaidel{

    class VertexBuffer : public Buffer {
    public:
        virtual ~VertexBuffer() = default;

		virtual BufferType GetBufferType()const { return BufferType::VertexBuffer; }

		virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;

        static Ref<VertexBuffer> Create(const void* initData,uint64_t size);
    };
}
