#include "KDpch.h"
#include "MaterialSerializer.h"
#include "Kaidel/Core/YAMLHelpers.h"
#include "Kaidel/Asset/AssetManager.h"

#include <yaml-cpp/yaml.h>

namespace Kaidel {

	namespace Utils {
		static const char* TextureChannelToString(StandardMaterialInstance::MaterialTextureChannel channel)
		{
			switch (channel)
			{
			case StandardMaterialInstance::MaterialTextureChannel::Red: return "Red";
			case StandardMaterialInstance::MaterialTextureChannel::Green: return "Green";
			case StandardMaterialInstance::MaterialTextureChannel::Blue: return "Blue";
			}

			KD_CORE_ASSERT(false);
			
			return nullptr;
		}

		static StandardMaterialInstance::MaterialTextureChannel StringToTextureChannel(const std::string& channel)
		{
			if ("Red")
			{
				return StandardMaterialInstance::MaterialTextureChannel::Red;
			}
			if ("Green")
			{
				return StandardMaterialInstance::MaterialTextureChannel::Green;
			}
			if ("Blue")
			{
				return StandardMaterialInstance::MaterialTextureChannel::Blue;
			}


			KD_CORE_ASSERT(false);
			
			return StandardMaterialInstance::MaterialTextureChannel::Red;
		}

		static const char* TextureTypeToString(StandardMaterialInstance::MaterialTextureType type)
		{
			switch (type)
			{
			case StandardMaterialInstance::MaterialTextureType::Albedo: return "Albedo";
			case StandardMaterialInstance::MaterialTextureType::Normal: return "Normal";
			case StandardMaterialInstance::MaterialTextureType::Metallic: return "Metallic";
			case StandardMaterialInstance::MaterialTextureType::Roughness: return "Roughness";
			case StandardMaterialInstance::MaterialTextureType::Emissive: return "Emissive";
			}

			KD_CORE_ASSERT(false);
			
			return nullptr;
		}

		static StandardMaterialInstance::MaterialTextureType StringToTextureType(const std::string& type)
		{
			if (type == "Albedo")
			{
				return StandardMaterialInstance::MaterialTextureType::Albedo;
			}

			if (type == "Normal")
			{
				return StandardMaterialInstance::MaterialTextureType::Normal;
			}

			if (type == "Metallic")
			{
				return StandardMaterialInstance::MaterialTextureType::Metallic;
			}

			if (type == "Roughness")
			{
				return StandardMaterialInstance::MaterialTextureType::Roughness;
			}

			if (type == "Emissive")
			{
				return StandardMaterialInstance::MaterialTextureType::Emissive;
			}

			KD_CORE_ASSERT(false);

			return StandardMaterialInstance::MaterialTextureType::Max;
		}
	}


	void MaterialSerializer::Serialize(const Path& path, Ref<StandardMaterialInstance> mat)
	{
		YAML::Emitter out;
		
		auto& uniformData = mat->GetUniformData();

		out << YAML::BeginMap;
		
		out << YAML::Key << "BaseColor" << YAML::Value << uniformData.BaseColor;
		
		out << YAML::Key << "Metalness" << YAML::Value << uniformData.Metalness;
		out << YAML::Key << "MetalnessChannel" << YAML::Value <<  Utils::TextureChannelToString(uniformData.MetalnessChannel);

		out << YAML::Key << "Roughness" << YAML::Value << uniformData.Roughness;
		out << YAML::Key << "RoughnessChannel" << YAML::Value << Utils::TextureChannelToString(uniformData.RoughnessChannel);

					
		for (uint32_t i = 0; i < (uint32_t)StandardMaterialInstance::MaterialTextureType::Max; ++i)
		{
			auto type = (StandardMaterialInstance::MaterialTextureType)i;
			
			const char* keyName = Utils::TextureTypeToString(type);

			Ref<Texture2D> texture = mat->GetTexture(type);

			if(!texture)
				continue;

			out << YAML::Key << keyName << YAML::Value << AssetManager::GetAssetMetadata(texture->GetHandle()).Path.string();
		}

		out << YAML::EndMap;


		std::ofstream file(path);

		file << out.c_str();
		file.close();
	}

	void MaterialSerializer::Deserialize(Ref<StandardMaterialInstance> mat, const Path& path)
	{
		YAML::Node node = YAML::Load(std::ifstream(path));

		auto& uniformData = mat->GetUniformData();

		uniformData.BaseColor = node["BaseColor"].as<glm::vec4>();
		
		uniformData.Metalness = node["Metalness"].as<float>();
		uniformData.MetalnessChannel = Utils::StringToTextureChannel(node["MetalnessChannel"].as<std::string>());

		uniformData.Roughness = node["Roughness"].as<float>();
		uniformData.RoughnessChannel = Utils::StringToTextureChannel(node["RoughnessChannel"].as<std::string>());

	}

}
