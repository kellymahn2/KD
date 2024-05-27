#pragma once


#include "VulkanBase.h"
#include "VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "PerFrameResource.h"

namespace Kaidel {
	namespace Vulkan {
		class VulkanVertexBuffer : public VertexBuffer {
		public:
			VulkanVertexBuffer(const VertexBufferSpecification& specification);

			~VulkanVertexBuffer();

			void Bind() const override {}

			void Unbind() const override {}

			void SetData(const void* data, uint32_t size) override;

			const BufferLayout& GetLayout() const override { return {}; }

			void SetLayout(const BufferLayout& layout) override {}

			const VulkanBuffer& GetBuffer()const { return *m_Buffers; }

		private:

			void ReadyStagingBuffer(const void* data,uint32_t size);

			void CreateBuffer(VulkanBuffer& buffer,uint32_t size) const;
			void DeleteBuffer(VulkanBuffer& buffer) const;

		private:
			VertexBufferSpecification m_Specification;
			//BufferLayout m_Layout{};

			PerFrameResource<VulkanBuffer> m_Buffers;
			VulkanBuffer m_StagingBuffer;
			void* m_MappedStagingBufferMemory = nullptr;
		};


		class VulkanIndexBuffer : public IndexBuffer {
		public:
			VulkanIndexBuffer(uint32_t* indices, uint32_t count);
			~VulkanIndexBuffer();
			void Bind() const override;

			void Unbind() const override;

			uint32_t GetCount() const override { return m_Count; }

			const VkBuffer& GetBuffer() { return m_Buffer; }

		private:
			uint32_t m_Count;
			VkBuffer m_Buffer;
			VkDeviceMemory m_Memory;
		};



	}
}
