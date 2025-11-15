#pragma once
#include "Kaidel/Core/Base.h"
#include "Buffer.h"

namespace Kaidel{
    class StorageBuffer : public Buffer{
    public:
        
        virtual ~StorageBuffer() = default;
		virtual BufferType GetBufferType()const { return BufferType::StorageBuffer; }
        virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) = 0;

		virtual void* Reserve(uint64_t dataSize) = 0;

		virtual void UploadReserve(const void* reservedStart, uint64_t dataSize) = 0;

        static Ref<StorageBuffer> Create(const void* data, uint64_t size);
    };
}
