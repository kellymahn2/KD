#include "KDPch.h"
#include "Sprite.h"

#include "Kaidel/Asset/AssetManager.h"

#include <Kaidel/Renderer/RendererGlobals.h>

namespace Kaidel
{

	SpriteSheet::SpriteSheet()
	{
		m_Sampler = RendererGlobals::GetSampler(SamplerFilter::Linear, SamplerMipMapMode::Linear, SamplerAddressMode::Repeat);
	}


	SpriteSheet::SpriteSheet(const SpriteSheet& rhs)
	{
		m_Sampler = rhs.m_Sampler;
		m_Texture = rhs.m_Texture;
		m_DataType = rhs.m_DataType;
		m_SpriteType = rhs.m_SpriteType;
		
		m_Sprites = rhs.m_Sprites;

		m_AssetHandle = rhs.m_AssetHandle;
	}


	SpriteSheet::SpriteSheet(Ref<Texture2D> texture, Ref<Sampler> sampler, SpriteDataType dataType /*= SpriteDataType::Color*/, SpriteType spriteType /*= SpriteType::Single*/)
		: m_Texture(texture), m_Sampler(sampler), m_DataType(dataType), m_SpriteType(spriteType)
	{}

	SpriteSheet::~SpriteSheet()
	{

	}


	void Sprite::RecalculateUVs(uint32_t width, uint32_t height)
	{
		float recWidth = 1.0f / width;
		float recHeight = 1.0f / height;

		UV0 = glm::vec2(Position.x * recWidth, (Position.y + Height) * recHeight);
		UV1 = glm::vec2((Position.x + Width) * recWidth, Position.y * recHeight);
	}

	Sprite::Sprite(uint32_t width, uint32_t height, uint32_t i, uint32_t j, float recWidth, float recHeight)
	{
		Width = width;
		Height = height;

		uint32_t top = j * height;
		uint32_t left = i * width;
		
		uint32_t bottom = top + height;
		uint32_t right = left + width;

		Position = glm::ivec2(left, top);

		UV0 = glm::vec2(Position.x * recWidth, (Position.y + Height) * recHeight);
		UV1 = glm::vec2((Position.x + Width) * recWidth, Position.y * recHeight);
	}

	void SpriteSheet::SplitByCellSize(uint32_t width, uint32_t height)
	{
		uint32_t textureWidth = m_Texture->GetTextureSpecification().Width;
		uint32_t textureHeight = m_Texture->GetTextureSpecification().Height;

		uint32_t countW = textureWidth / width;

		uint32_t countH = textureHeight / height;

		bool leftOverW = textureWidth % width != 0;

		bool leftOverH = textureHeight % height != 0;
		
		uint32_t stride = (countW + (uint32_t)leftOverW);

		uint32_t count = stride * (countH + (uint32_t)leftOverH);

		m_Sprites.resize(count);
		
		float recWidth = 1.0f / textureWidth;
		float recHeight = 1.0f / textureHeight;

		for (uint32_t j = 0; j < countH; ++j)
		{
			for (uint32_t i = 0; i < countW; ++i)
			{
				auto& sprite = m_Sprites[i + j * stride];

				sprite = Sprite(width, height, i, j, recWidth, recHeight);
			}
		}

		//remaining.
		if (leftOverW)
		{
			uint32_t w = textureWidth - countW * width;
			for (uint32_t i = 0; i < countH; ++i)
			{
				auto& sprite = m_Sprites[countW + i * stride];

				sprite = Sprite(w, height, countW, i, recWidth, recHeight);
			}
		}

		if (leftOverH)
		{
			uint32_t h = textureHeight - countH * height;
			for (uint32_t i = 0; i < countW; ++i)
			{
				auto& sprite = m_Sprites[i + countH * stride];

				sprite = Sprite(width, h, i, countH, recWidth, recHeight);
			}
		}

		//no one touches this.
		if (leftOverW && leftOverH)
		{
			uint32_t w = textureWidth - countW * width;
			uint32_t h = textureHeight - countH * height;
			
			m_Sprites.back() = Sprite(w, h, countW, countH, recWidth, recHeight);
		}

		SetNames();
	}

	void SpriteSheet::SplitByCellCount(uint32_t countX, uint32_t countY)
	{
		const auto& textureSpecs = m_Texture->GetTextureSpecification();

		uint32_t spriteWidth = textureSpecs.Width / countX;
		uint32_t spriteHeight = textureSpecs.Height / countY;

		float recWidth = 1.0f / textureSpecs.Width;
		float recHeight = 1.0f / textureSpecs.Height;

		m_Sprites.resize(countX * countY);

		for (uint32_t j = 0; j < countY; ++j)
		{
			for (uint32_t i = 0; i < countX; ++i)
			{
				m_Sprites[i + j * countX] = Sprite(spriteWidth, spriteHeight, i, j, recWidth, recHeight);
			}
		}

		SetNames();
	}


	uint64_t SpriteSheet::GetSubAssetCount() const
	{
		return m_Sprites.size();
	}

	void SpriteSheet::GetSubAssetAt(uint64_t i, SubAsset& subAsset)
	{
		KD_CORE_ASSERT(i < GetSubAssetCount());

		subAsset.Asset = this;
		subAsset.UserIndex = i;
	}

	void SpriteSheet::SetNames()
	{
		auto& name = AssetManager::GetAssetMetadata(GetHandle()).Path.stem();

		for (uint32_t i = 0; i < m_Sprites.size(); ++i)
		{
			m_Sprites[i].Name = fmt::format("{}_{}", name.string(), i);
		}
	}

}
