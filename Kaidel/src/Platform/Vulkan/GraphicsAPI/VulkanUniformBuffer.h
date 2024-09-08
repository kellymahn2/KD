#pragma once
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanUniformBuffer : public UniformBuffer {
	public:
		VulkanUniformBuffer(uint64_t size);
		~VulkanUniformBuffer();
		virtual RendererID GetRendererID()const { return (RendererID)&m_Buffer; }
		virtual void SetData(const void* data, uint64_t size, uint64_t offset) override;
	private:
		VulkanBackend::BufferInfo m_Buffer;
		friend class VulkanRendererAPI;
	};
}
