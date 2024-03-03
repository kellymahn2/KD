#include "KDpch.h"
#include "ProjectSerializer.h"

#include "Kaidel/Assets/AssetManager.h"
#include "Kaidel/Renderer/3D/MaterialSerializer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Kaidel {
	
	namespace Utils {

		static std::string KaidelAssetTypeToString(AssetType type) {
			switch (type)
			{
			case Kaidel::AssetType::None:return "None";
			case Kaidel::AssetType::Material:return "Material";
			case Kaidel::AssetType::Material2D:return "Material2D";
			}
			KD_CORE_ASSERT(false);
			return "";
		}

		static AssetType KaidelAssetTypeFromString(const std::string& type) {
			if (type == "Material")
				return AssetType::Material;
			if (type == "Material2D")
				return AssetType::Material2D;

			return AssetType::None;
		}

		static Ref<_Asset> DeserializeAsset(YAML::Node& assetNode){

			AssetType typeID = KaidelAssetTypeFromString(assetNode["AssetType"].as<std::string>());

			switch (typeID)
			{
			case Kaidel::AssetType::Material: {
				Ref<Material> mat = CreateRef<Material>();
				MaterialSerializer deserializer(mat);
				if (deserializer.Deserialize(assetNode["AbsolutePath"].as<std::string>())) {
					mat->AssetID(assetNode["UUID"].as<uint64_t>());
					return mat;
				}
			} 
			case Kaidel::AssetType::Material2D: {

			} break;
			}
			return {};
		}
	}

	

	ProjectSerializer::ProjectSerializer(Ref<Project> project) 
		: m_Project(project) 
	{}

	bool ProjectSerializer::Serialize(const std::filesystem::path& path) {
		YAML::Emitter out;
		const auto& config = m_Project->GetConfig();


		out << YAML::BeginMap;// Root

		out << YAML::Key << "Assets" << YAML::Value;

		{
			out << YAML::BeginMap; // Project Assets

			out << YAML::Key << "PhysicalAssets" << YAML::Value << YAML::BeginSeq;

			// Serialize physical assets

			{
				const auto& physical = AssetManager::m_PhysicalAssets;

				for (auto& [assetPath, asset] : physical) {
					out << YAML::BeginMap;
					out << YAML::Key << "UUID" << YAML::Value << asset->AssetID().operator size_t();
					out << YAML::Key << "AssetType" << YAML::Value << Utils::KaidelAssetTypeToString(asset->AssetTypeID());
					out << YAML::Key << "AbsolutePath" << YAML::Value << asset->Path().string();
					out << YAML::EndMap;
				}
			}

			out << YAML::EndSeq;

			out << YAML::EndMap; // Project Assets

		}



		out << YAML::Key << "Project" << YAML::Value;

		{
			out << YAML::BeginMap; // Project Settings
			out << YAML::Key << "Name" << YAML::Value << config.Name;
			out << YAML::Key << "StartScene" << YAML::Value << config.StartScene;
			out << YAML::Key << "RelativeAssetDirectory" << YAML::Value << config.RelAssetDirectory.string();
			out << YAML::Key << "ProjectAutoSave" << YAML::Value << config.ProjectAutoSave;
			out << YAML::Key << "ProjectAutoSaveTimer" << YAML::Value << config.ProjectAutoSaveTimer;

			out << YAML::EndMap; // Project Settings
		}

		out << YAML::EndMap; // Root

		std::filesystem::path p = path;
		if (FileSystem::directory_entry(p).is_directory()) {
			p /= config.Name;
		}
		std::ofstream file(p);
		file << out.c_str();
		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& path) {
		auto& config = m_Project->GetConfig();

		YAML::Node data;
		try {
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException e) {
			KD_CORE_ERROR("Failed to load project file at {}",path.string());
			return false;
		}

		auto projectNode = data["Project"];
		if (!projectNode) {
			KD_CORE_ERROR("File {} not a project file", path.string());
			return false;
		}

		auto assetsNode = data["Assets"];
		if (assetsNode) {
			// Deserialize physical assets
			{
				auto physicalAssets = assetsNode["PhysicalAssets"];
				if (physicalAssets) {
					for (auto assetNode : physicalAssets) {
						Ref<_Asset> asset = Utils::DeserializeAsset(assetNode);
						if (asset)
							AssetManager::Manage(asset);
					}
				}
			}
		}


		config.Name = projectNode["Name"].as<std::string>();
		config.StartScene = projectNode["StartScene"].as<std::string>();
		config.RelAssetDirectory = projectNode["RelativeAssetDirectory"].as<std::string>();
		config.AbsAssetDirectory = path.parent_path() / config.RelAssetDirectory;
		config.ProjectAutoSave = projectNode["ProjectAutoSave"].as<bool>();
		config.ProjectAutoSaveTimer = projectNode["ProjectAutoSaveTimer"].as<float>();
		return true;
	}

}
