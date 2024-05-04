#pragma once


#include "VulkanBase.h"

#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"


namespace Kaidel {
	namespace Vulkan {
		class VulkanVertexBuffer : public VertexBuffer {
		public:
			VulkanVertexBuffer(float* vertices, uint32_t size);
			VulkanVertexBuffer(uint32_t size);

			~VulkanVertexBuffer();

			void Bind() const override;

			void Unbind() const override;

			void SetData(const void* data, uint32_t size) override;

			const BufferLayout& GetLayout() const override;

			void SetLayout(const BufferLayout& layout) override;


		private:
			VkBuffer m_Buffer;
			VkDeviceMemory m_Memory;
			uint32_t m_MaxSize;
			BufferLayout m_Layout;
		};
	}
}
