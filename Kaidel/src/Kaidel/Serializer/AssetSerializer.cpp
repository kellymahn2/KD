#include "KDpch.h"
#include "AssetSerializer.h"


namespace Kaidel {

	bool AssetSerializer::Serialize(const Path& path){
		YAML::Emitter out;
		if (!SerializeTo(out))
			return false;

		std::ofstream file(path);

		if (!file.is_open())
			return false;

		file << out.c_str();
		return true;
	}
	bool AssetSerializer::Deserialize(const Path& path){

		std::ifstream file(path);
		if (!file.is_open())
			return false;
		YAML::Node node;
		try {
			node = YAML::Load(file);
		}
		catch(std::exception& e){
			KD_CORE_ASSERT(false, e.what());
			return false;
		}

		return DeserializeFrom(node);
	}
	bool AssetSerializer::SerializeTo(YAML::Emitter& out){
		out << YAML::BeginMap;
		out << YAML::Key << "ID" << YAML::Value << (uint64_t)m_Asset->AssetID();
		out << YAML::Key << "Name" << YAML::Value << m_Asset->Name();
		out << YAML::EndMap;
		return true;
	}
	bool AssetSerializer::DeserializeFrom(YAML::Node node) {
		{
			auto id = node["ID"];
			if (id) {
				m_Asset->AssetID(id.as<uint64_t>());
			}
		}

		{
			auto name = node["Name"];
			if (name) {
				m_Asset->Name(name.as<std::string>());
			}
		}
		return true;
	}
}
