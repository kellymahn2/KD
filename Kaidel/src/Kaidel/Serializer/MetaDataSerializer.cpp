#include "KDpch.h"
#include "MetaDataSerializer.h"
#include "AssetSerializer.h"
#include "Kaidel/Renderer/MaterialTexture.h"
#include "Kaidel/Renderer/3D/MaterialSerializer.h"
namespace Kaidel {


	bool MetaDataSerializer::Serialize(Ref<_Asset> asset,const Path& path) {

		YAML::Emitter out;


		AssetSerializer serializer(asset);
		if (!serializer.SerializeTo(out)) {
			return false;
		}

		out << YAML::BeginMap;
		out << YAML::Key << "Type" << YAML::Value << (uint64_t)asset->AssetTypeID();
		out << YAML::EndMap;

		std::ofstream file(path);
		if (file.is_open()) {
			file << out.c_str();
			return true;
		}

		return false;
	}
	Ref<_Asset> MetaDataSerializer::Deserialize(const Path& mtdPath,const Path& assetPath) {
		std::ifstream file(mtdPath);
		if (!file.is_open())
			return false;
		YAML::Node node = YAML::Load(file);

		
		try {
			AssetType type = (AssetType)node["Type"].as<uint64_t>();
			Ref<_Asset> asset;
			switch (type)
			{
			case Kaidel::AssetType::Material: {
				asset = CreateRef<Material>();
				MaterialSerializer deserializer(asset);
				if (!deserializer.Deserialize(assetPath)) {
					return {};
				}
			}break;
			case Kaidel::AssetType::MaterialTexture: {
				
				uint32_t slot = node["Slot"].as<uint32_t>();
				bool shouldFlip = node["ShouldFlip"].as<bool>();
				MaterialTexture::GetTextureArray()->InsertOrReplaceTexture(slot,assetPath.string(),shouldFlip);
				return {};
			}break;
			}

			AssetSerializer deserializer(asset);
			if (!deserializer.DeserializeFrom(node)) {
				return {};
			}
			return asset;
		}
		catch (std::exception& e) {
			KD_ERROR("{}", e.what());
			return {};
		}

	}
}
