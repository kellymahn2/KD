#pragma once
#include "Kaidel/Asset/AssetMetadata.h"

#include "yaml-cpp/yaml.h"

namespace Kaidel
{
	struct IDMap
	{
		std::unordered_map<uint64_t, UUID> IDs;

	};
}

namespace YAML
{
	template<>
	struct convert<::Kaidel::IDMap>
	{
		static Node encode(const ::Kaidel::IDMap& rhs)
		{
			Node node(NodeType::Map);
			for (const auto& [key, value] : rhs.IDs)
			{
				node[std::to_string(key)] = (uint64_t)value;
			}
			return node;
		}

		static bool decode(const Node& node, ::Kaidel::IDMap& rhs)
		{
			if (!node.IsMap())
				return false;

			rhs.IDs.clear();

			for (const auto& kv : node)
			{
				uint64_t key = 0;
				try
				{
					key = std::stoull(kv.first.as<std::string>());
				}
				catch (const std::exception&)
				{
					return false;
				}

				try
				{
					rhs.IDs.emplace(key, ::Kaidel::UUID(kv.second.as<uint64_t>()));
				}
				catch (const YAML::Exception&)
				{
					return false;
				}
			}
			return true;
		}
	};

	static YAML::Emitter& operator<<(YAML::Emitter& out, const ::Kaidel::IDMap& m)
	{
		out << YAML::BeginMap;

		for (auto& [fileID, id] : m.IDs)
		{
			out << YAML::Key << fileID << YAML::Value << (uint64_t)id;
		}

		out << YAML::EndMap;

		return out;
	}
}

namespace Kaidel
{
	

	class MetadataFile {
	public:
		MetadataFile(const Path& assetPath, bool discard = false)
			: m_AssetPath(assetPath), m_Discard(discard)
		{
			auto metadataPath = assetPath.string() + ".mtd";
			
			std::ifstream file;
			m_Emitter << YAML::BeginMap;

			if (GainAccessToFile(&file, metadataPath))
			{
				m_Node = YAML::Load(file);
				m_ID = m_Node["ID"].as<IDMap>();
				m_IsCreated = false;
				m_Emitter << YAML::Key << "ID" << YAML::Value << m_ID;
			}
		}

		~MetadataFile()
		{
			if(m_IsCreated || !m_Discard)
			{
				m_Emitter << YAML::Key << "ID" << YAML::Value << m_ID;
				m_Emitter << YAML::EndMap;

				const char* str = m_Emitter.c_str();

				auto metadataPath = m_AssetPath.string() + ".mtd";

				std::ofstream file(metadataPath);

				file << str;
			}
		}

		bool IsCreated() const { return m_IsCreated; }

		YAML::Node& GetNode() 
		{
			return m_Node;
		}

		YAML::Emitter& GetEmitter()
		{
			return m_Emitter;
		}

		void Resolve(AssetHandle& handle, uint64_t index = 0)
		{
			if (m_IsCreated)
			{
				m_ID.IDs[index] = handle;
			}
			else
			{
				handle = m_ID.IDs[index];
			}
		}
		
		auto& GetIDMap() { return m_ID; }

	private:
		YAML::Emitter m_Emitter;

		Path m_AssetPath;

		bool m_IsCreated = true;
		bool m_Discard;

		YAML::Node m_Node;

		IDMap m_ID;
	};
}
