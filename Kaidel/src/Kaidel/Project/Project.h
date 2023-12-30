#pragma once

#include "Kaidel/Core/Base.h"
#include <string>
#include <filesystem>
namespace Kaidel {
	struct ProjectConfig {
		std::string Name = "Untitled";
		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptPath;
		std::filesystem::path StartScene;
	};
	class Project {
	public:
		Project() {}
		static const std::filesystem::path& GetAssetDirectory() { 
			KD_CORE_ASSERT(s_ActiveProject); 
			return s_ActiveProject->m_Config.AssetDirectory; 
		}
		static const std::filesystem::path& GetProjectDirectory() {
			KD_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}
		static std::filesystem::path GetAssetPath(const std::filesystem::path& path) {
			KD_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / GetAssetDirectory() / path;
		}
		ProjectConfig& GetConfig() {
			return m_Config;
		}


		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
		static Ref<Project> GetActive() { return s_ActiveProject; }


	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;
		inline static Ref<Project> s_ActiveProject;
	};
}
