#include "KDpch.h"
#include "ProjectSerializer.h"

#include "Kaidel/Assets/AssetManager.h"
#include "Kaidel/Renderer/3D/MaterialSerializer.h"
#include "Kaidel/Renderer/MaterialTexture.h"
#include "Kaidel/Serializer/MetaDataSerializer.h"
#include "Kaidel/Serializer/TextureArraySerializer.h"
#include "Kaidel/Core/UsedFileExtensions.h"
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
			case Kaidel::AssetType::Texture2D:return "Texture2D";
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

		static Ref<_Asset> DeserializeAsset(YAML::Node& assetNode,const FileSystem::path& absolutePath){

			AssetType typeID = KaidelAssetTypeFromString(assetNode["AssetType"].as<std::string>());

			switch (typeID)
			{
			case Kaidel::AssetType::Material: {
				Ref<Material> mat = CreateRef<Material>();
				MaterialSerializer deserializer(mat);
				std::string relPath = assetNode["Path"].as<std::string>();
				std::string path = (absolutePath / relPath).string();
				if (deserializer.Deserialize(path)) {
					AssetManager::Manage(mat, true, path);
					return mat;
				}
			} break;
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

		//out << YAML::Key << "Assets" << YAML::Value;

		//{
		//	out << YAML::BeginMap; // Project Assets

		//	// Serialize physical assets

		//	out << YAML::Key << "PhysicalAssets" << YAML::Value << YAML::BeginSeq;

		//	{
		//		const auto& physical = AssetManager::m_PhysicalAssets;

		//		for (auto& [assetPath, asset] : physical) {
		//			out << YAML::BeginMap;
		//			out << YAML::Key << "AssetType" << YAML::Value << Utils::KaidelAssetTypeToString(asset->AssetTypeID());
		//			out << YAML::Key << "Path" << YAML::Value << assetPath.string();
		//			out << YAML::EndMap;
		//		}

		//	}

		//	out << YAML::EndSeq;

		//	/*out << YAML::Key << "MaterialTextures" << YAML::Value;
		//	TextureArraySerializer serializer(1);
		//	serializer.SerializeTo(MaterialTexture::GetTextureArray(), out);*/


		//	out << YAML::EndMap; // Project Assets
		//}



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
		catch (YAML::ParserException& e) {
			std::string s = e.what();
			KD_CORE_ERROR("Failed to load project file at {}",path.string());
			return false;
		}

		auto projectNode = data["Project"];
		if (!projectNode) {
			KD_CORE_ERROR("File {} not a project file", path.string());
			return false;
		}



		config.Name = projectNode["Name"].as<std::string>();
		config.StartScene = projectNode["StartScene"].as<std::string>();
		config.RelAssetDirectory = projectNode["RelativeAssetDirectory"].as<std::string>();
		config.AbsAssetDirectory = path.parent_path() / config.RelAssetDirectory;
		config.ProjectAutoSave = projectNode["ProjectAutoSave"].as<bool>();
		config.ProjectAutoSaveTimer = projectNode["ProjectAutoSaveTimer"].as<float>();
		m_Project->m_ProjectDirectory = path.parent_path();



		auto assetsNode = data["Assets"];
		if (assetsNode) {
			//// Deserialize physical assets
			//{
			//	auto physicalAssets = assetsNode["PhysicalAssets"];
			//	if (physicalAssets) {
			//		for (auto assetNode : physicalAssets) {
			//			Ref<_Asset> asset = Utils::DeserializeAsset(assetNode,path.parent_path());
			//		}
			//	}
			//}

			//// Deserialize textures
			//{
			//	auto materialTextures = assetsNode["MaterialTextures"];
			//	if (materialTextures) {
			//		TextureArraySerializer serializer(1);
			//		serializer.DeserializeFrom(MaterialTexture::GetTextureArray(),materialTextures);
			//	}
			//}
		}


		{
			auto rdi = FileSystem::recursive_directory_iterator(path.parent_path());

			std::map<uint32_t, std::string> assets;

			for (auto& de : rdi) {
				if (!de.is_directory()) {
					auto& path = de.path();
					if (path.extension()==Extensions::MetaData) {
						MetaDataSerializer mds;
						Path assetPath = "";
						{
							auto str = path.string();
							uint64_t index = str.find_last_of(".");
							assetPath = str.substr(0, index);
						}

						if (auto asset = mds.Deserialize(path,assetPath); asset) {
							AssetManager::Manage(asset, true, assetPath);
						}
					}
				}
			}

		}

	




		
		return true;
	}

}
