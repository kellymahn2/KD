#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "Buffer.h"

namespace Kaidel{

    class IndexBuffer : public Buffer {
    public:
        
        virtual ~IndexBuffer() = default;
		virtual BufferType GetBufferType()const { return BufferType::IndexBuffer; }
        static Ref<IndexBuffer> Create(const void* indices, uint64_t size, IndexType indexType);
    };
}
