#include "KDpch.h"
#include "VulkanGraphicsContext.h"
#include "VulkanSingleShader.h"
#include "VulkanRenderPass.h"
#include "VulkanBuffer.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"

#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"


#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

#include <GLFW/glfw3.h>




#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace Kaidel {

	Vulkan::VulkanGraphicsContext* Vulkan::VulkanGraphicsContext::s_GraphicsContext = nullptr;

//TODO: Destroy created resources.	




	static VkDescriptorPool descPool{};

	static Ref<Vulkan::VulkanCommandPool> commandPool;
	static Ref<Vulkan::VulkanCommandBuffer> commandBuffer;

	

	namespace Vulkan {

		// Function to create a descriptor pool
		VkDescriptorPool CreateDescriptorPool(VkDevice device) {
			VkDescriptorPool descriptorPool;

			VkDescriptorPoolSize poolSizes[] = {
				{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
				{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
				{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
				{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
			};

			VkDescriptorPoolCreateInfo poolInfo = {};
			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
			poolInfo.pPoolSizes = poolSizes;
			poolInfo.maxSets = 1000; // Adjust as needed

			if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create descriptor pool!");
			}

			return descriptorPool;
		}

		static VkRenderPass make_renderpass(VkDevice device, VkFormat swapchainImageFormat, bool debug) {

			VkRenderPass renderPass;

			// Color attachment description
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = swapchainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// Attachment reference
			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			// Subpass description
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			// Subpass dependency
			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			// Render pass create info
			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create render pass!");
			}

			return renderPass;

		}



		void ImGuiInit() {
			auto& graphicsContext = VulkanGraphicsContext::Get();

			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = VK_INSTANCE;
			init_info.PhysicalDevice = VK_PHYSICAL_DEVICE;
			init_info.Device = VK_DEVICE;
			init_info.QueueFamily = graphicsContext.m_QueueManager["GraphicsQueue"].FamilyIndex;
			init_info.Queue = graphicsContext.m_QueueManager["GraphicsQueue"].Queue;
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.DescriptorPool = descPool;
			init_info.MinImageCount = graphicsContext.m_Swapchain->GetSpecification().ImageCount;
			init_info.ImageCount = graphicsContext.m_Swapchain->GetSpecification().ImageCount;
			init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			init_info.Allocator = VK_ALLOCATOR_PTR;
			init_info.CheckVkResultFn = nullptr;
			ImGui_ImplVulkan_Init(&init_info, graphicsContext.m_Swapchain->GetSwapchainRenderPass());

			{
				// Use any command queue
				VkCommandPool command_pool = commandPool->GetCommandPool();
				VkCommandBuffer command_buffer = commandBuffer->GetCommandBuffer();

				vkResetCommandPool(graphicsContext.m_LogicalDevice, command_pool, 0);
				VkCommandBufferBeginInfo begin_info = {};
				begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				vkBeginCommandBuffer(command_buffer, &begin_info);

				ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

				VkSubmitInfo end_info = {};
				end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				end_info.commandBufferCount = 1;
				end_info.pCommandBuffers = &command_buffer;
				vkEndCommandBuffer(command_buffer);
				vkQueueSubmit(graphicsContext.m_QueueManager["GraphicsQueue"].Queue, 1, &end_info, VK_NULL_HANDLE);

				vkDeviceWaitIdle(graphicsContext.m_LogicalDevice);
				ImGui_ImplVulkan_DestroyFontUploadObjects();
			}

		}

		void ImGuiNewFrame() {
			ImGui_ImplVulkan_NewFrame();
		}
		void ImGuiRender(ImDrawData* drawData) {
			auto& graphicsContext = VulkanGraphicsContext::Get();

			uint32_t img = graphicsContext.CurrentImage;
			if (img == -1) {
				return;
			}

			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


			VkCommandBuffer commandBuffer = graphicsContext.m_Swapchain->GetFrames()[img].CommandBuffer;

			vkResetCommandBuffer(commandBuffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			vkBeginCommandBuffer(commandBuffer, &begin_info);


			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = graphicsContext.m_Swapchain->GetSwapchainRenderPass();
			renderPassInfo.framebuffer = graphicsContext.m_Swapchain->GetFrames()[img].Framebuffer;
			renderPassInfo.renderArea.offset.x = 0;
			renderPassInfo.renderArea.offset.y = 0;
			renderPassInfo.renderArea.extent = graphicsContext.m_Swapchain->GetSpecification().Extent;

			VkClearValue clearColor{};
			clearColor.color = { 0.5f,0.6f,.7f,1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

			vkCmdEndRenderPass(commandBuffer);
			vkEndCommandBuffer(commandBuffer);

			//Render
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			VkSemaphore waitSemaphores[] = { graphicsContext.m_Swapchain->GetFrames()[img].ImageAvailable->GetSemaphore() };
			VkPipelineStageFlags waitStages[] = { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			VkSemaphore renderFinished = graphicsContext.m_Swapchain->GetFrames()[img].RenderFinished->GetSemaphore();
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &renderFinished;

			vkQueueSubmit(graphicsContext.m_QueueManager["GraphicsQueue"].Queue, 1, &submitInfo, graphicsContext.m_Swapchain->GetFrames()[img].InFlightFence->GetFence());
		}
		void ImGuiShutdown() {
			ImGui_ImplVulkan_Shutdown();
		}


		namespace Utils {

			static std::string FilterDebugMessenge(const char* message) {
				return message;
			}

			static VkBool32 DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageTypes,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData)
			{
				switch (messageSeverity)
				{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: KD_CORE_INFO(FilterDebugMessenge(pCallbackData->pMessage)); break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:KD_CORE_WARN(FilterDebugMessenge(pCallbackData->pMessage)); break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:KD_CORE_ERROR(FilterDebugMessenge(pCallbackData->pMessage)); break;
				}
				return VK_FALSE;
			}
		}
		VulkanGraphicsContext::VulkanGraphicsContext(GLFWwindow* window)
			:m_Window(window)
		{
			KD_CORE_ASSERT(window, "Window handle is null!");
			s_GraphicsContext = this;
		}

		void VulkanGraphicsContext::Init()
		{
			bool isDebug = false;

#ifdef KD_DEBUG
			isDebug = true;
#endif // KD_DEBUG

			m_InstanceSpecification.ApplicationName = "Kaidel";
			//GLFW extensions
			{
				uint32_t glfwExtensionCount = 0;
				const char** glfwExtensions;
				glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

				m_InstanceSpecification.WantedInstanceExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
				if (isDebug) {
					m_InstanceSpecification.WantedInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
			}

			//Layers
			{
				if (isDebug) {
					m_InstanceSpecification.WantedInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
				}
			}

			//Instance creation
			auto instanceCreateResult = Utils::CreateInstance(m_InstanceSpecification);
			m_Instance = instanceCreateResult.Instance;
			m_VulkanAPIVersion = instanceCreateResult.VulkanAPIVersion;

			//Debug messenger creation
			auto debugMessengerCreateResult = Utils::CreateDebugMessenger(m_Instance,&Utils::DebugMessengerCallback);
			m_DebugMessenger = debugMessengerCreateResult.Messenger;

			//Surface creation
			auto surfaceCreateResult = Utils::CreateSurface(m_Instance, m_Window);
			m_Surface = surfaceCreateResult.Surface;

			m_PhysicalDeviceSpecification.WantedPhysicalDeviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			//Physical device choosing
			VulkanQueueGroupSpecification groupSpec{};

			{
				//Graphics Queue
				{
					VulkanQueueSpecification queueSpec{};
					queueSpec.Validator = [](const VkQueueFamilyProperties& props,uint32_t,VkPhysicalDevice physicalDevice) {
						return QueueValidateResult{ static_cast<bool>(props.queueFlags & VK_QUEUE_GRAPHICS_BIT) };
					};
					groupSpec.QueueSpecifications["GraphicsQueue"] = queueSpec;
				}
				//Present Queue
				{
					VulkanQueueSpecification queueSpec{};

					queueSpec.Validator = [surface = m_Surface](const VkQueueFamilyProperties& props,uint32_t familyIndex, VkPhysicalDevice physicalDevice) {
						VkBool32 supported = VK_TRUE;
						vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supported);
						return QueueValidateResult{ static_cast<bool>(supported) };
					};

					groupSpec.QueueSpecifications["PresentQueue"] = queueSpec;
				}

			}


			auto physicalDeviceChooseResult = Utils::ChoosePhysicalDevice(m_Instance,{groupSpec}, m_PhysicalDeviceSpecification);
			m_QueueManager = physicalDeviceChooseResult.QueueManager;
			m_PhysicalDevice = physicalDeviceChooseResult.PhysicalDevice;

			m_LogicalDeviceSpecification.Extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			if (isDebug) {
				m_LogicalDeviceSpecification.Layers.push_back("VK_LAYER_KHRONOS_validation");
			}

			//Logical device creation
			auto logicalDeviceCreateResult = Utils::CreateLogicalDevice(m_PhysicalDevice,m_QueueManager, m_LogicalDeviceSpecification);
			m_LogicalDevice = logicalDeviceCreateResult.LogicalDevice;

			gladLoaderLoadVulkan(m_Instance, m_PhysicalDevice, m_LogicalDevice);


			commandPool = CreateRef<VulkanCommandPool>(m_QueueManager["GraphicsQueue"].FamilyIndex);

			commandBuffer = CreateRef<VulkanCommandBuffer>(commandPool);

			descPool = CreateDescriptorPool(m_LogicalDevice);

			//Swapchain
			VulkanSwapchainSpecification swapchainSpecification{};
			swapchainSpecification.Extent = { 1280,720 };
			swapchainSpecification.ImageCount = 4;
			swapchainSpecification.LogicalDevice = m_LogicalDevice;
			swapchainSpecification.PhysicalDevice = m_PhysicalDevice;
			swapchainSpecification.PresentQueue = m_QueueManager["PresentQueue"];

			auto uniqueFamilies = m_QueueManager.GetUniqueFamilyIndices();
			swapchainSpecification.QueueFamiliesToShare = uniqueFamilies;
			if (swapchainSpecification.QueueFamiliesToShare.size() > 1) {
				swapchainSpecification.SharingMode = ImageSharingMode::Share;
			}
			else {
				swapchainSpecification.SharingMode = ImageSharingMode::NoShare;
			}
			swapchainSpecification.Surface = m_Surface;
			swapchainSpecification.SwapchainFormat = VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			swapchainSpecification.SwapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			
			swapchainSpecification.CommandPool = commandPool->GetCommandPool();

			m_Swapchain = CreateRef<VulkanSwapchain>(swapchainSpecification);

			m_UniqueQueueFamilyIndices = m_QueueManager.GetUniqueFamilyIndices();
			CurrentImage = -1;
			SwapBuffers();

			//MainCommandBuffer
			{
				VK_STRUCT(VkCommandBufferAllocateInfo, bufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
				bufferInfo.commandBufferCount = 1;
				bufferInfo.commandPool = commandPool->GetCommandPool();
				bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

				vkAllocateCommandBuffers(m_LogicalDevice, &bufferInfo, &m_MainCommandBuffer);
			}
		}

		void VulkanGraphicsContext::Shutdown() {

			//Swapchain
			m_Swapchain = nullptr;

			//Logical device
			vkDestroyDevice(m_LogicalDevice, VK_ALLOCATOR_PTR);
			//Surface
			vkDestroySurfaceKHR(m_Instance, m_Surface, VK_ALLOCATOR_PTR);
			//Debug messenger
			vkDestroyDebugUtilsMessengerEXT(m_Instance,m_DebugMessenger, VK_ALLOCATOR_PTR);
			//Instance
			vkDestroyInstance(m_Instance, VK_ALLOCATOR_PTR);
		}

		static uint64_t frames = 0;

		void VulkanGraphicsContext::OnResize(uint32_t width,uint32_t height)
		{
			m_Swapchain->Resize(width, height);
			CurrentImage = -1;
			CurrentImage = (CurrentImage + 1) % (m_Swapchain->GetSpecification().ImageCount);
			m_Swapchain->AcquireImage(CurrentImage);
		}

		void VulkanGraphicsContext::FlushCommandBuffers() {

			if (CurrentImage != -1) {
				//Present
				m_Swapchain->Present(m_Swapchain->GetLastAcquiredImage());
			}
			
		}

		void VulkanGraphicsContext::SwapBuffers()
		{
			FlushCommandBuffers();
			CurrentImage = (CurrentImage + 1) % (m_Swapchain->GetSpecification().ImageCount);
		    m_Swapchain->AcquireImage(CurrentImage);
		}
	}
}



