#include "KDpch.h"
#include "VulkanTexture2D.h"
#include "VulkanGraphicsContext.h"

namespace Kaidel {
	
	namespace Utils {
		static VkImageView CreateImageView(Format textureFormat, uint32_t levels, uint32_t layers, VkImage image) {
			VkImageView view{};
			VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.subresourceRange.aspectMask = Utils::IsDepthFormat(textureFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.layerCount = layers;
			viewInfo.subresourceRange.levelCount = levels;
			viewInfo.image = image;
			viewInfo.format = Utils::FormatToVulkanFormat(textureFormat);
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

			VK_ASSERT(vkCreateImageView(VK_DEVICE.GetDevice(), &viewInfo, nullptr, &view));
			return view;
		}

	}
	
	
	VulkanTexture2D::VulkanTexture2D(const Texture2DSpecification& spec)
		:m_Specification(spec)
	{
		m_Image = VK_ALLOCATOR.AllocateImage(spec.Width, spec.Height, 1, spec.Layers, VK_SAMPLE_COUNT_1_BIT, spec.Levels, spec.TextureFormat, ImageLayout::None,
												VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TYPE_2D);
		m_VulkanImage = (VkImage)m_Image._InternalImageID;
	}
	VulkanTexture2D::~VulkanTexture2D()
	{
	}
	Image& VulkanTexture2D::GetImage()
	{
		// TODO: insert return statement here
		return Image{};
	}
	void VulkanTexture2D::AddLayer(void* data, uint64_t size)
	{
		//TODO: implement.
	}

}
