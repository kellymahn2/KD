#include "KDpch.h"
#include "Texture.h"

#include "temp.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanTexture.h"
namespace Kaidel {
	TMAKE(Ref<Texture2D>, Texture2D::Create, TPACK(const Texture2DSpecification& specs), CreateRef<VulkanTexture2D>, TPACK(specs))
	
	TMAKE(Ref<TextureLayered>, TextureLayered::Create, TPACK(const TextureLayeredSpecification& specs), CreateRef<VulkanTextureLayered>, TPACK(specs))
}
