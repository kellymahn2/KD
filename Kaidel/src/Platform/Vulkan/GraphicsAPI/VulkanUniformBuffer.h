#pragma once


#include "VulkanBase.h"
#include "VulkanDefinitions.h"

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "PerFrameResource.h"

namespace Kaidel {
	namespace Vulkan {

		class VulkanUniformBuffer : public UniformBuffer {
		public:
			VulkanUniformBuffer(uint32_t size, uint32_t binding);
			~VulkanUniformBuffer();


			void SetData(const void* data, uint32_t size, uint32_t offset) override;

			void Bind(uint32_t binding) override;

			void Bind() override;

			void UnBind() override;

			uint32_t GetBindingIndex()const { return m_Binding; }
			const VulkanBuffer& GetBuffer()const { return *m_Buffer; }

		private:
			PerFrameResource<VulkanBuffer> m_Buffer;
			uint32_t m_Binding = 0;

		};


	}
}
