#pragma once
#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Backend.h"

namespace Kaidel{
    class VulkanVertexBuffer : public VertexBuffer{
    public:
		VulkanVertexBuffer(const void* initData, uint64_t size);
        ~VulkanVertexBuffer();
		
		virtual RendererID GetBackendID()const override { return (RendererID)&m_Buffer; }

        virtual void SetData(const void* data, uint64_t size, uint64_t offset) override;
		const VulkanBackend::BufferInfo& GetBufferInfo()const { return m_Buffer; }
    private:
		VulkanBackend::BufferInfo m_Buffer = {};
        friend class VulkanRendererAPI;
    };
}
