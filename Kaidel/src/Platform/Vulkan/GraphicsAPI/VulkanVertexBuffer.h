#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "PerFrameResource.h"

namespace Kaidel {
	class VulkanVertexBuffer : public VertexBuffer {
	public:

		VulkanVertexBuffer(uint32_t size);
		~VulkanVertexBuffer();
		void Bind() const override {}
		void Unbind() const override {}
		void SetData(const void* data, uint32_t size) override;
		const BufferLayout& GetLayout() const override;
		void SetLayout(const BufferLayout& layout) override;

		const VulkanBuffer& GetBuffer()const { return *m_Buffers; }

	private:
		PerFrameResource<VulkanBuffer> m_Buffers;
	};
}
