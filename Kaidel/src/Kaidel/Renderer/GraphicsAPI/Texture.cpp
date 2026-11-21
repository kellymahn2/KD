#include "KDpch.h"
#include "Texture.h"

#include "temp.h"

#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanTexture.h"
namespace Kaidel {

	TMAKE(Ref<Texture2D>, Texture2D::CreateUninitialized, TPACK(), CreateRef<VulkanTexture2D>, TPACK())
	TMAKE(Ref<Texture2D>, Texture2D::Create, TPACK(const Texture2DSpecification& specs), CreateRef<VulkanTexture2D>, TPACK(specs))

	Kaidel::Ref<Kaidel::Texture2D> Texture2D::CreateAt(Ref<Texture2D> texture, const Texture2DSpecification& specs)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::Vulkan: DynamicPointerCast<VulkanTexture2D>(texture)->Init(specs);
		}
		return texture;
	}

	TMAKE(Ref<TextureLayered>, TextureLayered::Create, TPACK(const TextureLayeredSpecification& specs), CreateRef<VulkanTextureLayered>, TPACK(specs))
	TMAKE(Ref<TextureReference>, TextureReference::Create, TPACK(const TextureReferenceSpecification& specs), CreateRef<VulkanTextureReference>, TPACK(specs))
	TMAKE(Ref<TextureCube>, TextureCube::Create, TPACK(const TextureCubeSpecification& specs), CreateRef<VulkanTextureCube>, TPACK(specs))
	TMAKE(Ref<TextureAtlas>, TextureAtlas::Create, TPACK(const TextureAtlasSpecification& specs), CreateRef<VulkanTextureAtlas>, TPACK(specs))
}
