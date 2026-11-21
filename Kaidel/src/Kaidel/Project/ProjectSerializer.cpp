#include "KDpch.h"
#include "ProjectSerializer.h"

#include "Kaidel/Core/UsedFileExtensions.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Kaidel {
	
	namespace Utils {

	}

	

	ProjectSerializer::ProjectSerializer(Ref<Project> project) 
		: m_Project(project) 
	{}

	bool ProjectSerializer::Serialize(const std::filesystem::path& path) {
		YAML::Emitter out;
		const auto& config = m_Project->GetConfig();

		out << YAML::BeginMap;// Root

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

		return true;
	}

}
