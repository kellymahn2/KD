#pragma once
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanUniformBuffer : public UniformBuffer {
	public:
		VulkanUniformBuffer(uint64_t size);
		~VulkanUniformBuffer();
		virtual RendererID GetRendererID()const { return (RendererID)&m_Buffer; }
		virtual RendererID GetBackendID()const override{ return (RendererID)&m_Buffer; }
		virtual void SetData(const void* data, uint64_t size, uint64_t offset) override;
		const VulkanBackend::BufferInfo& GetBufferInfo()const { return m_Buffer; }



	private:
		VulkanBackend::BufferInfo m_Buffer;
		friend class VulkanRendererAPI;
	};
}
