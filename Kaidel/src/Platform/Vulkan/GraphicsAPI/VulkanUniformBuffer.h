#pragma once

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "PerFrameResource.h"

namespace Kaidel {

	class VulkanUniformBuffer : public UniformBuffer {
	public:
		VulkanUniformBuffer(uint32_t size,uint32_t binding);
		~VulkanUniformBuffer();
		void SetData(const void* data, uint32_t size, uint32_t offset) override;
		void Bind(uint32_t binding) override {}
		void Bind() override {}
		void UnBind() override {}

		RendererID GetBufferID() const override { return (RendererID)m_Buffer->Buffer; }

	private:
		uint32_t m_Binding;
		uint32_t m_Size;
		PerFrameResource<VulkanBuffer> m_Buffer;
	};

}
