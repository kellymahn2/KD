#pragma once

#include "VulkanBase.h"
#include "VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/Image.h"

namespace Kaidel {
	namespace Vulkan {
		class VulkanFramebufferImage : public FramebufferImage{
		public:
			VulkanFramebufferImage(VkImage image, VkDeviceMemory imageMemory, VkImageView imageView, 
									VkSampler sampler, VkDescriptorSet descriptorSet,VkDescriptorPool descriptorPool,ImageLayout currentLayout)
				:FramebufferImage((ImageShaderID)imageView,currentLayout),m_Image(image),
					m_ImageMemory(imageMemory),m_ImageView(imageView),m_Sampler(sampler),m_DescriptorSet(descriptorSet),m_DescriptorPool(descriptorPool)
			{}

			~VulkanFramebufferImage();

			void TransitionLayout(ImageLayout finalLayout) override;
		private:
			static VkImageLayout GetVulkanImageLayout(ImageLayout layout);
			static ImageLayout GetKaidelImageLayout(VkImageLayout layout);
		private:
			VkImage m_Image;
			VkDeviceMemory m_ImageMemory;
			VkImageView m_ImageView;
			VkSampler m_Sampler;
			VkDescriptorSet m_DescriptorSet;
			VkDescriptorPool m_DescriptorPool;
		};
	}
}
