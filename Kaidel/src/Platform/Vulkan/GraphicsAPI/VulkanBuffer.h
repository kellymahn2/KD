#pragma once


#include "VulkanBase.h"

#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"


namespace Kaidel {
	namespace Vulkan {
		class VulkanVertexBuffer : public VertexBuffer {
		public:
			VulkanVertexBuffer(const VertexBufferSpecification& specification);

			~VulkanVertexBuffer();

			void Bind() const override;

			void Unbind() const override;

			void SetData(const void* data, uint32_t size) override;

			const BufferLayout& GetLayout() const override{ return m_Layout; }

			void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
		private:
			void DestroyCurrentBuffer();

			void CreateVertexBuffer(const void* values = nullptr);

		private:
			VertexBufferSpecification m_Specification;
			BufferLayout m_Layout{};

			VkBuffer m_Buffer = VK_NULL_HANDLE;
			VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		};


		class VulkanIndexBuffer : public IndexBuffer {
		public:
			VulkanIndexBuffer(uint32_t* indices, uint32_t count);
			~VulkanIndexBuffer();
			void Bind() const override;

			void Unbind() const override;

			uint32_t GetCount() const override { return m_Count; }
		private:
			uint32_t m_Count;
			VkBuffer m_Buffer;
			VkDeviceMemory m_Memory;
		};



	}
}
