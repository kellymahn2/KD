#include "KDpch.h"

#include "MaterialTexture.h"

namespace Kaidel {

	void MaterialTexture::Init() {
		s_MaterialTextureArray = Texture2DArray::Create(1024, 1024, TextureFormat::RGBA8);
		uint32_t default = 0xffffffff;
		s_MaterialTextureArray->PushTexture(&default, 1, 1);
	}
	uint32_t MaterialTexture::LoadTexture(const std::filesystem::path& texturePath,bool shouldFlip){
		return s_MaterialTextureArray->PushTexture(texturePath.string(), shouldFlip);
	}
	uint32_t MaterialTexture::LoadTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip){
		return s_MaterialTextureArray->PushTexture(data, width, height, shouldFlip);
	}
}
