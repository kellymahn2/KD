#include "KDpch.h"
#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Kaidel {
	
	ProjectSerializer::ProjectSerializer(Ref<Project> project) 
		: m_Project(project) 
	{}

	bool ProjectSerializer::Serialize(const std::filesystem::path& path) {
		YAML::Emitter out;
		const auto& config = m_Project->GetConfig();


		out << YAML::BeginMap;// Root
		out << YAML::Key << "Project" << YAML::Value;


		{
			out << YAML::BeginMap; // Project
			out << YAML::Key << "Name" << YAML::Value << config.Name;
			out << YAML::Key << "StartScene" << YAML::Value << config.StartScene.string();
			out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
			out << YAML::Key << "ScriptPath" << YAML::Value << config.ScriptPath.string();
			out << YAML::EndMap; // Project
		}



		out << YAML::EndMap; // Root

		std::ofstream file(path);
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


		config.Name = projectNode["Name"].as<std::string>();
		config.StartScene = projectNode["StartScene"].as<std::string>();
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.ScriptPath = projectNode["ScriptPath"].as<std::string>();
		return true;
	}

}
