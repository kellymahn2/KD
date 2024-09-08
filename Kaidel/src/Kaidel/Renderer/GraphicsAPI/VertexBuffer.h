#pragma once
#include "Kaidel/Core/Base.h"

namespace Kaidel{

    class VertexBuffer : public IRCCounter<false> {
    public:
        virtual ~VertexBuffer() = default;

        virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;

        static Ref<VertexBuffer> Create(uint64_t size);
    };
}
