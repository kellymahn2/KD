#pragma once
#include "Kaidel/Renderer/GraphicsAPI/StorageBuffer.h"
#include "Backend.h"

namespace Kaidel{
    class VulkanStorageBuffer : public StorageBuffer{
    public:
        VulkanStorageBuffer(uint64_t size);
        ~VulkanStorageBuffer();

		virtual RendererID GetRendererID()const { return (RendererID)&m_Buffer; }
		virtual void SetData(const void* data,uint64_t size,uint64_t offset) override;
    private:
		VulkanBackend::BufferInfo m_Buffer;
		friend class VulkanRendererAPI;
    };
}
