#pragma once
#include "GraphicsAPI/TextureArray.h"
#include "Kaidel/Assets/AssetManager.h"
namespace Kaidel {
	class MaterialTexture{
	public:
		MaterialTexture(uint32_t slot)
			:m_Slot(slot)
		{
		}

		static void Init();
		static uint32_t LoadTexture(const std::filesystem::path& texturePath, bool shouldFlip = true);
		static uint32_t LoadTexture(void* data, uint32_t width, uint32_t height, bool shouldFlip = true);
		static Ref<Texture2DArray> GetTextureArray() { return s_MaterialTextureArray; }
	private:
		uint32_t m_Slot;

		static inline Ref<Texture2DArray> s_MaterialTextureArray;
	};
}
