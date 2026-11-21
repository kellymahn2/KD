#include "KDPch.h"

#include "SpriteSheetSerializer.h"

#include "Kaidel/Core/YAMLHelpers.h"
#include "Kaidel/Core/EnumStringConvertor.h"
#include "Kaidel/Renderer/RendererGlobals.h"


namespace Kaidel
{
	namespace Utils
	{
		DECL_CONVERTOR(FilterToString, StringToFilter,
			SamplerFilter::Nearest, "Nearest",
			SamplerFilter::Linear, "Linear");
		DECL_CONVERTOR(WrapToString, StringToWrap,
			SamplerAddressMode::Repeat, "Repeat",
			SamplerAddressMode::MirroredRepeat, "Mirror",
			SamplerAddressMode::ClampToEdge, "Clamp"
			);
	}
	
	void SpriteSheetSerializer::Serialize(Ref<SpriteSheet> sheet, MetadataFile* mtdFile)
	{
		YAML::Emitter& out = mtdFile->GetEmitter();

		auto& sampler = sheet->GetSampler()->GetSamplerState();

		out << YAML::Key << "Filter" << YAML::Value << Utils::FilterToString(sampler.MagFilter);
		out << YAML::Key << "Wrap" << YAML::Value << Utils::WrapToString(sampler.AddressModeU);

		out << YAML::Key << "Sprites" << YAML::Value << YAML::BeginSeq;

		for (auto& sprite : sheet->m_Sprites)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Name" << YAML::Value << sprite.Name;
			out << YAML::Key << "Width" << YAML::Value << sprite.Width;
			out << YAML::Key << "Height" << YAML::Value << sprite.Height;
			out << YAML::Key << "Position" << YAML::Value << sprite.Position;

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
	}

	void SpriteSheetSerializer::Deserialize(Ref<SpriteSheet> sheet, Ref<Texture2D> texture, MetadataFile* mtdFile)
	{
		sheet->m_Texture = texture;

		uint32_t width = texture->GetTextureSpecification().Width, height = texture->GetTextureSpecification().Height;

		if (mtdFile->IsCreated())
		{
			return;
		}

		auto& node = mtdFile->GetNode();

		SamplerFilter filter = Utils::StringToFilter(node["Filter"].as<std::string>());
		SamplerAddressMode addressMode = Utils::StringToWrap(node["Wrap"].as<std::string>());

		sheet->SetSampler(RendererGlobals::GetSampler(filter, SamplerMipMapMode::Linear, addressMode));

		auto sprites = node["Sprites"];

		if (!sprites)
		{
			return;
		}

		uint32_t spriteCount = std::distance(sprites.begin(), sprites.end());

		sheet->m_Sprites.resize(spriteCount);

		auto spriteIt = sheet->m_Sprites.begin();

		for (auto& spriteNode : sprites)
		{
			auto& sprite = (*spriteIt++);

			sprite = Sprite();

			sprite.Name = spriteNode["Name"].as<std::string>();
			sprite.Width = spriteNode["Width"].as<uint32_t>();
			sprite.Height = spriteNode["Height"].as<uint32_t>();
			sprite.Position = spriteNode["Position"].as<glm::vec2>();

			sprite.RecalculateUVs(width, height);
		}
	}

}
