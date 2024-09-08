#pragma once
#include "Kaidel/Core/Base.h"
#include "ShaderInputBuffer.h"

namespace Kaidel{
    class StorageBuffer : public ShaderInputBuffer{
    public:
        
        virtual ~StorageBuffer() = default;

        virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;

        static Ref<StorageBuffer> Create(uint64_t size);
    };
}
