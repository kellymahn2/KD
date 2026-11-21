#include "IconLibrary.h"

#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

namespace Kaidel {
	static Ref<TextureAtlas> s_Atlas;
	static Ref<DescriptorSet> s_Set;
	static std::unordered_map<std::string, uint32_t> s_Icons;

	void IconLibrary::Init(const Path& path)
	{

		TextureAtlasSpecification atlasSpecs;
		atlasSpecs.Width = 640;
		atlasSpecs.Height = 640;
		atlasSpecs.Format = Format::RGBA8UN;
		atlasSpecs.Layout = ImageLayout::ShaderReadOnlyOptimal;
		atlasSpecs.Mipped = false;
		s_Atlas = TextureAtlas::Create(atlasSpecs);

		FileSystem::recursive_directory_iterator it(path);

		for (auto& de : it)
		{
			const auto& p = de.path();
			auto extension = p.extension().string();

			if (extension == ".png" ||
				extension == ".jpg" ||
				extension == ".jpeg")
			{
				auto data = TextureLibrary::LoadData(p, Format::RGBA8UN);

				auto key = FileSystem::relative(p, path).string();
				std::replace(key.begin(), key.end(), '\\', '/');

				s_Icons[key] =
					s_Atlas->AddToAtlas(data.Data.get(), data.Width, data.Height);
			}
		}

		DescriptorSetLayoutSpecification specs({ {DescriptorType::SamplerWithTexture, ShaderStage_FragmentShader} });
		s_Set = DescriptorSet::Create(specs);

		s_Set->Update(s_Atlas, GLOBAL_SAMPLER_LINEAR, ImageLayout::ShaderReadOnlyOptimal, 0);
	}

	void IconLibrary::Shutdown()
	{
		s_Atlas = {};
		s_Icons = {};
		s_Icons.clear();
	}

	uint32_t IconLibrary::GetIcon(const std::string& name)
	{
		return s_Icons.at(name);
	}

	Kaidel::Ref<Kaidel::TextureAtlas> IconLibrary::GetAtlas()
	{
		return s_Atlas;
	}

	Kaidel::Ref<Kaidel::DescriptorSet> IconLibrary::GetDescriptorSet()
	{
		return s_Set;
	}
}
