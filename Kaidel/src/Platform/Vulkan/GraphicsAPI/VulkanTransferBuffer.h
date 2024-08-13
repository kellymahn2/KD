#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"


namespace Kaidel {
	class VulkanTransferBuffer : public TransferBuffer {
	public:
		VulkanTransferBuffer(uint64_t size, const void* initData, uint64_t initDataSize);
		~VulkanTransferBuffer();

		void* Map() override;

		void Unmap() override;

		void SetDataFromAllocation(const void* allocation, uint64_t size, uint64_t offset) override;

		void GetData(void* out, uint64_t size, uint64_t offset) override;

		const VulkanBuffer& GetBuffer()const { return m_Buffer; }

	private:
		VulkanBuffer m_Buffer;
	};
}
