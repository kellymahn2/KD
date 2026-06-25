#pragma once
#include "Kaidel/Asset/AssetMetadata.h"

#include "yaml-cpp/yaml.h"

#include <variant>

namespace Kaidel
{
	class MetadataFile {
	public:
		MetadataFile(const Path& assetPath)
			: m_AssetPath(assetPath)
		{
			auto metadataPath = assetPath.string() + ".mtd";
			
			std::ifstream file;
			if (GainAccessToFile(&file, metadataPath))
			{
				m_Node = YAML::Load(file);
				m_Iterator = m_Node.begin();
				m_IsCreated = false;
			}
			else
			{
				m_Emitter << YAML::BeginMap;
			}
		}

		~MetadataFile()
		{
			if (m_IsCreated)
			{
				m_Emitter << YAML::EndMap;

				const char* str = m_Emitter.c_str();

				auto metadataPath = m_AssetPath.string() + ".mtd";

				std::ofstream file(metadataPath);

				file << str;
			}
		}

		void SerializeHandle(AssetHandle handle)
		{
			m_Emitter << YAML::Key << m_Index << YAML::Value << handle.Get();
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

		void Resolve(AssetHandle& handle)
		{
			if (IsCreated())
			{
				SerializeHandle(handle);
			}
			else
			{
				handle = AssetHandle(m_Iterator->second.as<uint64_t>());
				++m_Iterator;
			}
		}

	private:
		YAML::Emitter m_Emitter;
		uint32_t m_Index = 0;

		Path m_AssetPath;

		bool m_IsCreated = true;


		YAML::Node::const_iterator m_Iterator;
		YAML::Node m_Node;
	};
}
