#pragma once
#include "GraphicsAPI\Texture.h"
#include "GraphicsAPI\Sampler.h"

#include <glm\glm.hpp>

namespace Kaidel
{
	enum class SpriteDataType
	{
		Color, Normal, Data
	};

	enum class SpriteType
	{
		Single, Multiple
	};

	struct Sprite
	{ 
		std::string Name;
		//Top-Left
		glm::ivec2 Position = glm::ivec2(0, 0);
		uint32_t Width = 0, Height = 0;
		//Bottom-left, top-right
		glm::vec2 UV0 = glm::vec2(0.0f), UV1 = glm::vec2(0.0f);

		void RecalculateUVs(uint32_t width, uint32_t height);

		Sprite() = default;

		Sprite(uint32_t width, uint32_t height, uint32_t i, uint32_t j, float recWidth, float recHeight);
	};

	class SpriteSheet : public AssetBase
	{
	public:
		SpriteSheet();

		SpriteSheet(Ref<Texture2D> texture, Ref<Sampler> sampler, SpriteDataType dataType = SpriteDataType::Color, SpriteType spriteType = SpriteType::Single);

		virtual ~SpriteSheet();

		SpriteSheet(const SpriteSheet& rhs);

		void SplitByCellSize(uint32_t width, uint32_t height);
		void SplitByCellCount(uint32_t countX, uint32_t countY);

		std::vector<Sprite>& GetSprites()
		{
			return m_Sprites;
		}

		Ref<Texture2D> GetTexture() const { return m_Texture; }
		Ref<Sampler> GetSampler() const { return m_Sampler; }

		DECL_ASSET_TYPE(SpriteSheet);

		uint64_t GetSubAssetCount() const override;

		void GetSubAssetAt(uint64_t i, SubAsset& subAsset) override;

		void SetSampler(Ref<Sampler> sampler) { m_Sampler = sampler; }

	private:
		void SetNames();
	private:
		SpriteDataType m_DataType;
		SpriteType m_SpriteType;

		Ref<Texture2D> m_Texture;
		std::vector<Sprite> m_Sprites;

		Ref<Sampler> m_Sampler;

		friend class SpriteSheetSerializer;
	};
}

