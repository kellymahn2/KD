#include "KDpch.h"
#include "VulkanConstants.h"


namespace Kaidel {
	namespace Utils {
		VkFormat KaidelTextureFormatToVkFormat(TextureFormat format) {
			switch (format)
			{
			case Kaidel::TextureFormat::RGBA32F:return VK_FORMAT_R32G32B32A32_SFLOAT;
			case Kaidel::TextureFormat::RGBA32UI: return VK_FORMAT_R32G32B32A32_UINT;
			case Kaidel::TextureFormat::RGBA32I:return VK_FORMAT_R32G32B32A32_SINT;
			case Kaidel::TextureFormat::RGB32F:return VK_FORMAT_R32G32B32_SFLOAT;
			case Kaidel::TextureFormat::RGB32UI:return VK_FORMAT_R32G32B32_UINT;
			case Kaidel::TextureFormat::RGB32I:return VK_FORMAT_R32G32B32_SINT;
			case Kaidel::TextureFormat::RGBA16F:return VK_FORMAT_R16G16B16A16_SFLOAT;
			case Kaidel::TextureFormat::RGBA16UI:return VK_FORMAT_R16G16B16A16_UINT;
			case Kaidel::TextureFormat::RGBA16NORM:return VK_FORMAT_R16G16B16A16_SNORM;
			case Kaidel::TextureFormat::RGBA16I:return VK_FORMAT_R16G16B16A16_SINT;
			case Kaidel::TextureFormat::RG32F:return VK_FORMAT_R32G32_SFLOAT;
			case Kaidel::TextureFormat::RG32UI:return VK_FORMAT_R32G32_UINT;
			case Kaidel::TextureFormat::RG32I:return VK_FORMAT_R32G32_SINT;
			case Kaidel::TextureFormat::RGBA8:return VK_FORMAT_R8G8B8A8_UNORM;
			case Kaidel::TextureFormat::RGBA8UI:return VK_FORMAT_R8G8B8A8_UINT;
			case Kaidel::TextureFormat::RGBA8NORM:return VK_FORMAT_R8G8B8A8_SNORM;
			case Kaidel::TextureFormat::RGBA8I:return VK_FORMAT_R8G8B8A8_SINT;
			case Kaidel::TextureFormat::RG16F:return VK_FORMAT_R16G16_SFLOAT;
			case Kaidel::TextureFormat::RG16UI:return VK_FORMAT_R16G16_UINT;
			case Kaidel::TextureFormat::RG16NORM:return VK_FORMAT_R16G16_SNORM;
			case Kaidel::TextureFormat::RG16I:return VK_FORMAT_R16G16_SINT;
			case Kaidel::TextureFormat::R32F:return VK_FORMAT_R32_SFLOAT;
			case Kaidel::TextureFormat::R32UI:return VK_FORMAT_R32_UINT;
			case Kaidel::TextureFormat::R32I:return VK_FORMAT_R32_SINT;
			case Kaidel::TextureFormat::RG8:return VK_FORMAT_R8G8_UNORM;
			case Kaidel::TextureFormat::RG8UI:return VK_FORMAT_R8G8_UINT;
			case Kaidel::TextureFormat::RG8NORM:return VK_FORMAT_R8G8_SNORM;
			case Kaidel::TextureFormat::RG8I:return VK_FORMAT_R8G8_SINT;
			case Kaidel::TextureFormat::R16F:return VK_FORMAT_R16_SFLOAT;
			case Kaidel::TextureFormat::R16UI:return VK_FORMAT_R16_UINT;
			case Kaidel::TextureFormat::R16NORM:return VK_FORMAT_R16_SNORM;
			case Kaidel::TextureFormat::R16I:return VK_FORMAT_R16_SINT;
			case Kaidel::TextureFormat::R8:return VK_FORMAT_R8_UNORM;
			case Kaidel::TextureFormat::R8UI:return VK_FORMAT_R8_UINT;
			case Kaidel::TextureFormat::R8NORM:return VK_FORMAT_R8_SNORM;
			case Kaidel::TextureFormat::R8I:return VK_FORMAT_R8_SINT;
			case Kaidel::TextureFormat::Depth16:return VK_FORMAT_D16_UNORM;
			case Kaidel::TextureFormat::Depth32F:return VK_FORMAT_D32_SFLOAT;
			case Kaidel::TextureFormat::Depth24Stencil8:return VK_FORMAT_D24_UNORM_S8_UINT;
			}
			KD_CORE_ASSERT(false, "Unknown format");
			return VK_FORMAT_UNDEFINED;
		}
	}
}
