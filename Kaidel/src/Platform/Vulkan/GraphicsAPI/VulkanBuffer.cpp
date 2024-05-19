#include "KDpch.h"
#include "VulkanBuffer.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {


	namespace Utils {

		struct BufferCreateResult {
			VkBuffer Buffer = VK_NULL_HANDLE;
			VkDeviceMemory AllocatedMemory = VK_NULL_HANDLE;
		};
		struct BufferSpecification {
			VkDevice LogicalDevice;
			VkPhysicalDevice PhysicalDevice;
			VkMemoryPropertyFlags MemoryPropertyFlags;
			VkBufferUsageFlags BufferUsage;
			uint32_t Size;
			const void* InitialData = nullptr;
		};

		static uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice,uint32_t supportedMemoryIndices,VkMemoryPropertyFlags requestedProperties) {
			VkPhysicalDeviceMemoryProperties props{};
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &props);

			for (uint32_t i = 0; i < props.memoryTypeCount; ++i) {
				bool supported = (supportedMemoryIndices & (1 << i));
				bool sufficient = (props.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties;

				if (supported && sufficient)
					return i;

			}

			return -1;
		}


		static BufferCreateResult CreateBuffer(const BufferSpecification& specification) {
			BufferCreateResult result{};

			const auto& uniqueIndices = VK_UNIQUE_INDICES;

			VK_STRUCT(VkBufferCreateInfo, bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
			bufferInfo.pQueueFamilyIndices = uniqueIndices.data();
			bufferInfo.queueFamilyIndexCount = (uint32_t)uniqueIndices.size();
			bufferInfo.sharingMode = bufferInfo.queueFamilyIndexCount > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			bufferInfo.size = specification.Size;
			bufferInfo.usage = specification.BufferUsage;
			
			VK_ASSERT(vkCreateBuffer(specification.LogicalDevice, &bufferInfo, VK_ALLOCATOR_PTR, &result.Buffer));

			VkMemoryRequirements bufferMemReq{};
			vkGetBufferMemoryRequirements(specification.LogicalDevice, result.Buffer, &bufferMemReq);

			VK_STRUCT(VkMemoryAllocateInfo, memoryInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
			memoryInfo.allocationSize = bufferMemReq.size;
			memoryInfo.memoryTypeIndex = FindMemoryTypeIndex(specification.PhysicalDevice, bufferMemReq.memoryTypeBits, specification.MemoryPropertyFlags);
			VK_ASSERT(vkAllocateMemory(specification.LogicalDevice, &memoryInfo, VK_ALLOCATOR_PTR, &result.AllocatedMemory));


			if (specification.InitialData) {
				void* mapData = nullptr;
				VK_ASSERT(vkMapMemory(specification.LogicalDevice, result.AllocatedMemory, 0, specification.Size,0, &mapData));
				memcpy(mapData, specification.InitialData, specification.Size);
				vkUnmapMemory(specification.LogicalDevice, result.AllocatedMemory);
				//VK_ASSERT(vkBindBufferMemory(specification.LogicalDevice, result.Buffer, result.AllocatedMemory, 0));
			}
			VK_ASSERT(vkBindBufferMemory(specification.LogicalDevice, result.Buffer, result.AllocatedMemory, 0));


			return result;
		}

		static void CopyBuffer(VkBuffer src,VkBuffer dst,VkDeviceSize copySize, VkDevice device,VkCommandBuffer commandBuffer,VkQueue transferQueue) {

			vkResetCommandBuffer(commandBuffer, 0);
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(commandBuffer, &beginInfo);
			VkBufferCopy copy{};
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = copySize;
			vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copy);
			vkEndCommandBuffer(commandBuffer);

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;
			vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(transferQueue);
		}

		static void DeleteBuffer(VkDevice device,VkBuffer buffer, VkDeviceMemory memory) {
			vkDestroyBuffer(device, buffer, VK_ALLOCATOR_PTR);
			vkFreeMemory(device, memory, VK_ALLOCATOR_PTR);
		}


	}


	namespace Vulkan {
		
		VulkanVertexBuffer::VulkanVertexBuffer(const VertexBufferSpecification& specification)
			:m_Specification(specification)
		{
			CreateVertexBuffer(m_Specification.Data);
		}

		void VulkanVertexBuffer::CreateVertexBuffer(const void* values)
		{

			Utils::BufferCreateResult stagingBuffer{};


			{
				Utils::BufferSpecification stagingBufferSpecification{};
				stagingBufferSpecification.BufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingBufferSpecification.InitialData = values;
				stagingBufferSpecification.LogicalDevice = VK_DEVICE;
				stagingBufferSpecification.PhysicalDevice = VK_PHYSICAL_DEVICE;
				stagingBufferSpecification.Size = m_Specification.Size;
				stagingBufferSpecification.MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				stagingBuffer = Utils::CreateBuffer(stagingBufferSpecification);
			}


			Utils::BufferSpecification bufferSpec{};
			bufferSpec.BufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferSpec.Size = m_Specification.Size;
			bufferSpec.InitialData = nullptr;
			bufferSpec.LogicalDevice = VK_DEVICE;
			bufferSpec.PhysicalDevice = VK_PHYSICAL_DEVICE;
			bufferSpec.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


			Utils::BufferCreateResult buffer = Utils::CreateBuffer(bufferSpec);

			Utils::CopyBuffer(stagingBuffer.Buffer, buffer.Buffer, m_Specification.Size, VK_DEVICE, VK_MAIN_COMMAND_BUFFER, VK_DEVICE_QUEUE("GraphicsQueue"));

			m_Buffer = buffer.Buffer;
			m_Memory = buffer.AllocatedMemory;

			//Delete staging buffer
			Utils::DeleteBuffer(VK_DEVICE, stagingBuffer.Buffer, stagingBuffer.AllocatedMemory);

		}

		VulkanVertexBuffer::~VulkanVertexBuffer()
		{
			if(m_Buffer)
				DestroyCurrentBuffer();
		}
		void VulkanVertexBuffer::Bind() const
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(VK_CURRENT_IMAGE.CommandBuffer, 0, 1, &m_Buffer, &offset);
		}
		void VulkanVertexBuffer::Unbind() const
		{
		}
		void VulkanVertexBuffer::SetData(const void* data, uint32_t size)
		{
			if (size > m_Specification.Size) {
				if (m_Buffer) {
					DestroyCurrentBuffer();
				}
				
				m_Specification.Size = size;
				CreateVertexBuffer(data);
				return;
			}

			Utils::BufferCreateResult stagingBuffer{};


			{
				Utils::BufferSpecification stagingBufferSpecification{};
				stagingBufferSpecification.BufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingBufferSpecification.InitialData = data;
				stagingBufferSpecification.LogicalDevice = VK_DEVICE;
				stagingBufferSpecification.PhysicalDevice = VK_PHYSICAL_DEVICE;
				stagingBufferSpecification.Size = size;
				stagingBufferSpecification.MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				stagingBuffer = Utils::CreateBuffer(stagingBufferSpecification);
			}

			Utils::CopyBuffer(stagingBuffer.Buffer, m_Buffer, size, VK_DEVICE, VK_MAIN_COMMAND_BUFFER, VK_DEVICE_QUEUE("GraphicsQueue"));

			//Delete staging buffer
			Utils::DeleteBuffer(VK_DEVICE, stagingBuffer.Buffer, stagingBuffer.AllocatedMemory);

		}
		void VulkanVertexBuffer::DestroyCurrentBuffer()
		{
			Utils::DeleteBuffer(VK_DEVICE, m_Buffer, m_Memory);
		}
		
		VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* indices, uint32_t count)
		{
			Utils::BufferCreateResult stagingBuffer{};


			{
				Utils::BufferSpecification stagingBufferSpecification{};
				stagingBufferSpecification.BufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingBufferSpecification.InitialData = indices;
				stagingBufferSpecification.LogicalDevice = VK_DEVICE;
				stagingBufferSpecification.PhysicalDevice = VK_PHYSICAL_DEVICE;
				stagingBufferSpecification.Size = count * sizeof(uint32_t);
				stagingBufferSpecification.MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				stagingBuffer = Utils::CreateBuffer(stagingBufferSpecification);
			}


			Utils::BufferSpecification bufferSpec{};
			bufferSpec.BufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferSpec.Size = count * sizeof(uint32_t);
			bufferSpec.InitialData = nullptr;
			bufferSpec.LogicalDevice = VK_DEVICE;
			bufferSpec.PhysicalDevice = VK_PHYSICAL_DEVICE;
			bufferSpec.MemoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


			Utils::BufferCreateResult buffer = Utils::CreateBuffer(bufferSpec);

			Utils::CopyBuffer(stagingBuffer.Buffer, buffer.Buffer, count * sizeof(uint32_t), VK_DEVICE, VK_MAIN_COMMAND_BUFFER, VK_DEVICE_QUEUE("GraphicsQueue"));

			m_Buffer = buffer.Buffer;
			m_Memory = buffer.AllocatedMemory;

			//Delete staging buffer
			Utils::DeleteBuffer(VK_DEVICE, stagingBuffer.Buffer, stagingBuffer.AllocatedMemory);
		}
		VulkanIndexBuffer::~VulkanIndexBuffer()
		{
			vkDestroyBuffer(VK_DEVICE, m_Buffer, VK_ALLOCATOR_PTR);
			vkFreeMemory(VK_DEVICE, m_Memory, VK_ALLOCATOR_PTR);
		}
		void VulkanIndexBuffer::Bind() const
		{
			vkCmdBindIndexBuffer(VK_CURRENT_IMAGE.CommandBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
		}
		void VulkanIndexBuffer::Unbind() const
		{
		}

	}
}
