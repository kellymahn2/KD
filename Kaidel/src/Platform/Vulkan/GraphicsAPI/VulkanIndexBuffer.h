#pragma once
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Backend.h"


namespace Kaidel{
    class VulkanIndexBuffer : public IndexBuffer{
    public:
        VulkanIndexBuffer(const void* indices, uint64_t size, IndexType type);
        ~VulkanIndexBuffer();

		virtual RendererID GetBackendID()const override { return (RendererID)&m_Buffer; }

		const VulkanBackend::BufferInfo& GetBufferInfo()const { return m_Buffer; }
		IndexType GetIndexType()const { return m_IndexType; }
    private:
        VulkanBackend::BufferInfo m_Buffer;
        IndexType m_IndexType;
        friend class VulkanRendererAPI;
    };
}
