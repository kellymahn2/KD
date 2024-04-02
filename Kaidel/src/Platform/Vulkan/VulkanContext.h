#pragma once
#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"


#define VULKAN_GRAPHICS_QUEUE_NAME "GraphicsQueue"
#define VULKAN_PRESENT_QUEUE_NAME "PresentQueue"




#define GLAD_VULKAN_IMPLEMENTATION
#define GLAD_INCLUDE_WIN32
#include <glad/vulkan.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#include <optional>

#include <unordered_set>

struct GLFWwindow;
#define GET_CONTEXT(p) auto p = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext().get());
namespace Kaidel {

	class VulkanContext : public GraphicsContext {
	public:
		VulkanContext(GLFWwindow* context);

		void Init() override;
		void SwapBuffers() override;
		void Shutdown()override;

	private:
		void InitPhysicalDevice();
		void InitLogicalDevice();

	private:
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		
		
		GLFWwindow* m_Context;
	};


}
