#pragma once


#include "Kaidel/Core/Base.h"
#include "Constants.h"

struct VkAttachmentDescription;

namespace Kaidel {

	namespace Vulkan {
		struct VulkanImageHandle {
			void* Image;
			uint32_t ImageWidth;
			uint32_t ImageHeight;
			TextureFormat ImageFormat;
			VkAttachmentDescription* ImageDescription;
		};
		struct VulkanFramebufferImageHandle {
			void* Framebuffer;
			uint64_t ImageIndexInFramebuffer;
			VulkanImageHandle ImageHandle;
		};
	}

	namespace OpenGL {
		struct OpenGLImageHandle {
			uint32_t Image;
			uint32_t ImageWidth;
			uint32_t ImageHeight;
			TextureFormat ImageFormat;
		};
		struct OpenGLFramebufferImageHandle {
			uint32_t Framebuffer;
			uint64_t ImageIndexInFramebuffer;
			OpenGLImageHandle ImageHandle;
		};
	}


	struct ImageHandle {
		Vulkan::VulkanImageHandle VK;
		OpenGL::OpenGLImageHandle OGL;
	};

	union FramebufferImageHandle {
		Vulkan::VulkanFramebufferImageHandle VK;
		OpenGL::OpenGLFramebufferImageHandle OGL;
	};

}

