#include "KDpch.h"
#include "VulkanGraphicsContext.h"
#include "VulkanSingleShader.h"
#include "VulkanRenderPass.h"
#include "VulkanBuffer.h"
#include "Initialization/VulkanInstance.h"
#include "Initialization/VulkanPhysicalDevice.h"
#include "Initialization/VulkanLogicalDevice.h"

#include "VulkanCommandPool.h"
#include "VulkanCommandBuffer.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "VulkanMemory.h"
#include "VulkanRendererAPI.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>

namespace Kaidel {

	Vulkan::VulkanGraphicsContext* Vulkan::VulkanGraphicsContext::s_GraphicsContext = nullptr;

//TODO: Destroy created resources.	

	static VkDescriptorPool descPool{};

	static Ref<Vulkan::VulkanCommandPool> commandPool;



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

				VulkanCommandBuffer commandBuffer = VulkanCommandBuffer(commandPool);

				VkCommandBuffer command_buffer = commandBuffer.GetCommandBuffer();

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


			VkCommandBufferBeginInfo begin_info = {};
			begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


			VkCommandBuffer commandBuffer = graphicsContext.m_Swapchain->GetFrames()[graphicsContext.m_AcquiredImage].CommandBuffer;

			vkResetCommandBuffer(commandBuffer,0);
			vkBeginCommandBuffer(commandBuffer, &begin_info);


			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = graphicsContext.m_Swapchain->GetSwapchainRenderPass();
			renderPassInfo.framebuffer = graphicsContext.m_Swapchain->GetFrames()[graphicsContext.m_AcquiredImage].Framebuffer;
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
			
		}
		void ImGuiShutdown() {
			ImGui_ImplVulkan_Shutdown();
		}


		VulkanGraphicsContext::VulkanGraphicsContext(GLFWwindow* window)
			:m_Window(window),m_AcquiredImage(0),m_MaxFramesInFlight(0),m_CurrentFrame(0)
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
			#pragma region Instance creation
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
			#pragma endregion

			//Debug messenger creation
			auto debugMessengerCreateResult = Utils::CreateDebugMessenger(m_Instance,&Utils::DebugMessengerCallback);
			m_DebugMessenger = debugMessengerCreateResult.Messenger;

			//Surface creation
			auto surfaceCreateResult = Utils::CreateSurface(m_Instance, m_Window);
			m_Surface = surfaceCreateResult.Surface;

			m_PhysicalDeviceSpecification.WantedPhysicalDeviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};


			#pragma region Physical and logical device
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
				//Transfer Queue
				{
					VulkanQueueSpecification queueSpec{};
					queueSpec.Validator = [](const VkQueueFamilyProperties& props, uint32_t, VkPhysicalDevice physicalDevice) {
						return QueueValidateResult{ static_cast<bool>(props.queueFlags & VK_QUEUE_TRANSFER_BIT) };
						};
					groupSpec.QueueSpecifications["TransferQueue"] = queueSpec;
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
			#pragma endregion

			commandPool = CreateRef<VulkanCommandPool>(m_QueueManager["GraphicsQueue"].FamilyIndex);

			descPool = CreateDescriptorPool(m_LogicalDevice);

			#pragma region Swapchain
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

			#pragma endregion

			m_UniqueQueueFamilyIndices = m_QueueManager.GetUniqueFamilyIndices();

			//MainCommandBuffer
			{
				VK_STRUCT(VkCommandBufferAllocateInfo, bufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
				bufferInfo.commandBufferCount = 1;
				bufferInfo.commandPool = commandPool->GetCommandPool();
				bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

				vkAllocateCommandBuffers(m_LogicalDevice, &bufferInfo, &m_MainCommandBuffer);
			}

			//Global graphics command buffer
			{
				VK_STRUCT(VkCommandBufferAllocateInfo, bufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
				bufferInfo.commandBufferCount = m_Swapchain->GetSpecification().ImageCount;
				bufferInfo.commandPool = commandPool->GetCommandPool();
				bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				
				m_GraphicsCommandBuffers.resize(bufferInfo.commandBufferCount);

				VK_ASSERT(vkAllocateCommandBuffers(m_LogicalDevice, &bufferInfo, m_GraphicsCommandBuffers.data()));
			}

			m_TransferCommandPool = CreateRef<VulkanCommandPool>(GetQueue("TransferQueue").FamilyIndex,false);
			m_GraphicsCommandPool = CreateRef<VulkanCommandPool>(GetQueue("GraphicsQueue").FamilyIndex, false);


			{
				Utils::BufferSpecification spec{};
				spec.BufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				spec.LogicalDevice = m_LogicalDevice;
				spec.MemoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
				spec.PhysicalDevice = m_PhysicalDevice;
				spec.QueueFamilies = { GetQueue("TransferQueue").FamilyIndex };
				spec.Size = GlobalStagingBufferSize;
				Utils::BufferCreateResult result = Utils::CreateBuffer(spec);
				m_GlobalStagingBuffer = VulkanBuffer(result.Buffer, result.AllocatedMemory, GlobalStagingBufferSize);
				m_GlobalStagingBuffer.Map(m_LogicalDevice);
			}

			/*m_GraphicsCommandBuffer->BeginRecording(0);

			CurrentImage = -1;
			SwapBuffers();*/
			m_MaxFramesInFlight = m_Swapchain->GetSpecification().ImageCount;

			for (auto& frame : m_Swapchain->GetFrames()) {
				frame.Task.TaskConditionVariable = std::make_shared<std::condition_variable>();
				frame.Task.TaskSynchronizationMutex = std::make_shared<std::mutex>();
				frame.Task.TaskWorkerThread = std::make_shared<std::thread>([this,&frame]() {
					while (!frame.Task.StopWorkerThread) {
						std::unique_lock<std::mutex> lock(*frame.Task.TaskSynchronizationMutex);

						frame.Task.TaskConditionVariable->wait(lock, [&frame]() {return frame.Task.TasksReady || frame.Task.StopWorkerThread; });

						if (frame.Task.StopWorkerThread)
							break;
						frame.InFlightFence->Wait();

						while (!frame.Task.Tasks.empty()) {
							frame.Task.Tasks.front()();
							frame.Task.Tasks.pop();
						}

						frame.Task.TasksReady = false;
					}
				});
			}

			StartSwapchain();

			//SwapBuffers();
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

		void VulkanGraphicsContext::OnResize(uint32_t width,uint32_t height)
		{
			/*m_Swapchain->Resize(width, height);
			CurrentImage = -1;
			CurrentImage = (CurrentImage + 1) % (m_Swapchain->GetSpecification().ImageCount);
			m_Swapchain->AcquireImage(CurrentImage);*/

			m_Swapchain->Resize(width, height);
			StartSwapchain();
		}

		void VulkanGraphicsContext::StartSwapchain()
		{
			m_CurrentFrame = 0;
			m_AcquiredImage = m_Swapchain->AcquireImage(GetCurrentFrame().InFlightFence->GetFence(), VK_NULL_HANDLE, GetCurrentFrame().ImageAvailable->GetSemaphore());
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_ASSERT(vkBeginCommandBuffer(m_GraphicsCommandBuffers[0], &beginInfo));
		}

		void VulkanGraphicsContext::Present()
		{
			//m_Swapchain->Present();

			//m_Swapchain->Present({GetCurrentFrame().RenderFinished->GetSemaphore()},CurrentImage);
		}

		void VulkanGraphicsContext::FlushCommandBuffers() {

			//if (CurrentImage != -1) {

			//	//Present
			//	m_Swapchain->Present(CurrentImage);
			//}

			
		}

		void VulkanGraphicsContext::SwapBuffers()
		{

			auto& frames = m_Swapchain->GetFrames();

			VkSemaphore waitSemaphores[] = { frames[m_CurrentFrame].ImageAvailable->GetSemaphore()};
			VkSemaphore signalSemaphores[] = { frames[m_CurrentFrame].RenderFinished->GetSemaphore()};


			VK_ASSERT(vkEndCommandBuffer(GetGraphicsCommandBuffer()));
			

			VkCommandBuffer commandBuffers[] = { frames[m_AcquiredImage].CommandBuffer ,GetGraphicsCommandBuffer()};

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			VkPipelineStageFlags waitStages[] = { VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = waitSemaphores;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = ARRAYSIZE(commandBuffers);
			submitInfo.pCommandBuffers = commandBuffers;

			submitInfo.signalSemaphoreCount = ARRAYSIZE(signalSemaphores);
			submitInfo.pSignalSemaphores = signalSemaphores;

			VK_ASSERT(vkQueueSubmit(GetQueue("GraphicsQueue").Queue, 1, &submitInfo, GetCurrentFrame().InFlightFence->GetFence()));

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = signalSemaphores;
			VkSwapchainKHR swapchains[] = { m_Swapchain->GetSwapchain() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapchains;
			presentInfo.pImageIndices = &m_AcquiredImage;
			
			vkQueuePresentKHR(GetQueue("PresentQueue"), &presentInfo);
			m_Swapchain->GetFrames()[m_CurrentFrame].Task.TasksReady = true;
			m_Swapchain->GetFrames()[m_CurrentFrame].Task.TaskConditionVariable->notify_one();

			m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;

			frames[m_CurrentFrame].InFlightFence->Wait();
			std::unique_lock<std::mutex> lock(*frames[m_CurrentFrame].Task.TaskSynchronizationMutex);
			frames[m_CurrentFrame].InFlightFence->Reset();

			vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain->GetSwapchain(), UINT64_MAX, frames[m_CurrentFrame].ImageAvailable->GetSemaphore(), VK_NULL_HANDLE, &m_AcquiredImage);

			VK_ASSERT(vkResetCommandBuffer(GetGraphicsCommandBuffer(), 0));
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			VK_ASSERT(vkBeginCommandBuffer(GetGraphicsCommandBuffer(),&beginInfo));

		}
	}
}



