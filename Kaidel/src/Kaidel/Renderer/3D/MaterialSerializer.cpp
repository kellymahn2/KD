#include "KDpch.h"
#include "MaterialSerializer.h"
#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Kaidel {

	static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	
	void MaterialSerializer::Serialize(YAML::Emitter& out) {
		out << YAML::BeginMap;
		out << YAML::Key << "Color" << YAML::Value << m_Material->GetColor();
		out << YAML::Key << "Diffuse" << YAML::Value << m_Material->GetDiffuse();
		out << YAML::Key << "Specular" << YAML::Value << m_Material->GetSpecular();
		out << YAML::Key << "Shininess" << YAML::Value << m_Material->GetShininess();
		out << YAML::EndMap;
	}

	bool MaterialSerializer::Serialize(const FileSystem::path& path) {
		YAML::Emitter out;
		Serialize(out);
		std::ofstream file(path);
		if (!file.is_open())
			return false;
		file << out.c_str();
		return true;
	}
	bool MaterialSerializer::Deserialize(const FileSystem::path& path){
		std::ifstream file(path);
		if (!file.is_open())
			return false;
		YAML::Node material = YAML::Load(file);

		if (material["Color"]) {
			m_Material->SetColor(material["Color"].as<glm::vec4>());
		}
		if (material["Diffuse"]) {
			m_Material->SetDiffuse(material["Diffuse"].as<uint32_t>());
		}
		if (material["Specular"]) {
			m_Material->SetSpecular(material["Specular"].as<uint32_t>());
		}
		if (material["Shininess"]) {
			m_Material->SetShininess(material["Shininess"].as<float>());
		}

		m_Material->Path(path);

		return true;
	}
}
