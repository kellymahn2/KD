#include "KDpch.h"

#include "AssetSerializer.h"
#include "Kaidel/Renderer/3D/Material.h"
#include "Kaidel/Renderer/3D/MaterialSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Kaidel {

	

	static Ref<_Asset> DeserializePhysicalAsset(YAML::Node& node) {

		YAML::Node uuidNode = node["UUID"];
		if (!uuidNode)
			return {};
		UUID id = uuidNode.as<uint64_t>();

		YAML::Node absolutePathNode = node["AbsolutePath"];
		if (!absolutePathNode)
			return {};
		std::string absolutePathString = absolutePathNode.as<std::string>();
		FileSystem::path assetPath = absolutePathString;

		if (!FileSystem::exists(assetPath))
			return {};

		if (id == 0)
			return {};

		std::string extension = assetPath.extension().string();

		if (extension == ".mat") {
			Ref<Material> mat = CreateRef<Material>();
			mat->AssetID(id);
			mat->Path(assetPath);
			MaterialSerializer serializer(mat);
			if (!serializer.Deserialize(assetPath))
				return {};
			return mat;
		}

	}

	static void SerializeMaterial(Ref<Material> mat,YAML::Emitter& emitter) {
		MaterialSerializer serializer(mat);
		emitter << YAML::Key << "Values" << YAML::Value;
		serializer.Serialize(emitter);
	}


	bool AssetSerializer::Serialize(const FileSystem::path& path) {
		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "PhysicalAssets" << YAML::Value << YAML::BeginSeq;

		// Serialize physical assets

		{
			const auto& physical = AssetManager::m_PhysicalAssets;

			for (auto& [assetPath, asset] : physical) {
				out << YAML::BeginMap;
				out << YAML::Key << "UUID" << YAML::Value << asset->AssetID().operator size_t();
				out << YAML::Key << "AbsolutePath" << YAML::Value << asset->Path().string();
				out << YAML::EndMap;
			}
		}

		out << YAML::EndSeq;
		
		out << YAML::Key << "NonPhysicalAssets" << YAML::Value << YAML::BeginSeq;

		// Serialize non-physical assets

		{
			const auto& nonPhysical = AssetManager::m_NonPhysicalAssets;

			for (auto& [uuid, asset] : nonPhysical) {
				out << YAML::BeginMap;
				out << YAML::Key << "UUID" << YAML::Value << asset->AssetID().operator size_t();
				AssetFunctionApplier applier(asset);
				applier.Apply<Material>(SerializeMaterial, out);
				out << YAML::EndMap;
			}
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;

		std::ofstream file(path);
		if (!file.is_open())
			return false;
		file << out.c_str();
		return true;

	}
	bool AssetSerializer::Deserialize(const FileSystem::path& path) {
		if (!FileSystem::directory_entry(path).exists()) {
			KD_CORE_ERROR("Could not load asset definition file at {}", path.string());
			return false;
		}
		std::ifstream file(path);
		
		YAML::Node data = YAML::Load(file);
		
		// Serialize physical assets
		{
			auto physicalAssets = data["PhysicalAssets"];
			if (physicalAssets) {
				for (auto assetNode : physicalAssets) {
					Ref<_Asset> asset = DeserializePhysicalAsset(assetNode);
					if(asset)
						AssetManager::Manage(asset);
				}
			}
		}
		return true;
	}

}

