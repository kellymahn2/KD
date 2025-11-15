#include "KDpch.h"
#include "Texture.h"

#include "temp.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanTexture.h"
namespace Kaidel {
	TMAKE(Ref<Texture2D>, Texture2D::Create, TPACK(const Texture2DSpecification& specs), CreateRef<VulkanTexture2D>, TPACK(specs))

	TMAKE(Ref<TextureLayered>, TextureLayered::Create, TPACK(const TextureLayeredSpecification& specs), CreateRef<VulkanTextureLayered>, TPACK(specs))
	TMAKE(Ref<TextureReference>, TextureReference::Create, TPACK(const TextureReferenceSpecification& specs), CreateRef<VulkanTextureReference>, TPACK(specs))
	TMAKE(Ref<TextureCube>, TextureCube::Create, TPACK(const TextureCubeSpecification& specs), CreateRef<VulkanTextureCube>, TPACK(specs))

}
