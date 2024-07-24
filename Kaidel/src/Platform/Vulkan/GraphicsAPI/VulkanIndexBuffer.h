#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"


namespace Kaidel {

	class VulkanIndexBuffer : public IndexBuffer {
	public:

		VulkanIndexBuffer(uint32_t* indices, uint32_t count);
		~VulkanIndexBuffer();
		void Bind() const override;

		void Unbind() const override;

		uint32_t GetCount() const override { return 0; }

		const VulkanBuffer& GetBuffer()const { return m_IndexBuffer; }
	private:
		VulkanBuffer m_IndexBuffer;
	};

}
