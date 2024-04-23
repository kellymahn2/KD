#pragma once


#include "VulkanBase.h"

#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"


#include <vector>
#include <string>
#include <optional>
#include <set>

struct GLFWwindow;

namespace Kaidel {

	
	namespace Vulkan {
	
		struct InstanceSpecification {
			std::vector<const char*> WantedInstanceExtensions;
			std::vector<const char*> WantedInstanceLayers;
			std::string ApplicationName;
		};

		struct PhysicalDeviceSpecification {
			std::vector<const char*> WantedPhysicalDeviceExtensions;
		};

		struct QueueFamilyIndices {
			std::optional<uint32_t> GraphicsQueueFamilyIndex, PresentQueueFamilyIndex, TransferQueueFamilyIndex, ComputeQueueFamilyIndex;
			bool Valid() const {
				return GraphicsQueueFamilyIndex.has_value() && PresentQueueFamilyIndex.has_value();
			}
			operator bool()const {
				return Valid();
			}
		};

		struct DeviceQueues {
			VkQueue GraphicsQueue = VK_NULL_HANDLE;
			VkQueue PresentQueue = VK_NULL_HANDLE;
			VkQueue TransferQueue = VK_NULL_HANDLE;
			VkQueue ComputeQueue = VK_NULL_HANDLE;
		};

		struct LogicalDeviceSpecification {
			std::vector<const char*> Extensions;
			std::vector<const char*> Layers;
			VkPhysicalDeviceFeatures Features;
		};


		class VulkanGraphicsContext : public GraphicsContext{
		public:
			VulkanGraphicsContext(GLFWwindow* window);

			void Init() override;
			void SwapBuffers() override;
			void Shutdown() override;
		private:
			GLFWwindow* m_Window = nullptr;

			//Instance
			InstanceSpecification m_InstanceSpecification = {};
			uint32_t m_VulkanAPIVersion = 0;
			VkInstance m_Instance = VK_NULL_HANDLE;

			//Surface
			VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
			
			//PhysicalDevice
			PhysicalDeviceSpecification m_PhysicalDeviceSpecification = {};
			QueueFamilyIndices m_QueueFamilyIndices = {};
			VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

			//LogicalDevice
			LogicalDeviceSpecification m_LogicalDeviceSpecification = {};
			DeviceQueues m_DeviceQueues = {};
			VkDevice m_LogicalDevice = VK_NULL_HANDLE;

		};
	}

}

