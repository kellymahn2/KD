#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"

namespace Kaidel{

    class IndexBuffer : public IRCCounter<false> {
    public:
        
        virtual ~IndexBuffer() = default;

        static Ref<IndexBuffer> Create(const void* indices, uint64_t size, IndexType indexType);
    };
}
