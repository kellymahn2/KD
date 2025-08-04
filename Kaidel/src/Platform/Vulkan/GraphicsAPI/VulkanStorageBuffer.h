#pragma once
#include "Kaidel/Renderer/GraphicsAPI/StorageBuffer.h"
#include "Backend.h"

namespace Kaidel{
    class VulkanStorageBuffer : public StorageBuffer{
    public:
        VulkanStorageBuffer(const void* data, uint64_t size);
        ~VulkanStorageBuffer();
		virtual RendererID GetBackendID()const override { return (RendererID)&m_Buffer; }

		virtual RendererID GetRendererID()const { return (RendererID)&m_Buffer; }
		virtual void SetData(const void* data,uint64_t size,uint64_t offset) override;
		const VulkanBackend::BufferInfo& GetBufferInfo()const { return m_Buffer; }

    private:
		VulkanBackend::BufferInfo m_Buffer;
		friend class VulkanRendererAPI;
    };
}
