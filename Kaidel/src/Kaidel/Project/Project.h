#pragma once

#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/IRCP.h"
#include <string>
#include <filesystem>
#include <vector>
#include <unordered_set>
namespace Kaidel {
	struct ProjectConfig {
		std::string Name = "Untitled";
		std::filesystem::path RelAssetDirectory;
		std::filesystem::path AbsAssetDirectory;
		std::string StartScene;

		bool ProjectAutoSave = true;
		bool SceneAutoSave = true;
		float ProjectAutoSaveTimer = 60.0f,TimeSinceLastProjectAutoSave = 0.0f;
		float SceneAutoSaveTimer = 30.0f, TimeSinceLastSceneAutoSave = 0.0f;
	};
	class Project : public IRCCounter<false> {
	public:
		Project() {}
		// ProjectConfig::RelAssetDirectory
		static const std::filesystem::path& GetRelativeAssetDirectory() { 
			KD_CORE_ASSERT(GetActive()); 
			return GetActive()->m_Config.RelAssetDirectory;
		}
		// ProjectConfig::AbsAssetDirectory
		static const std::filesystem::path& GetAbsoluteAssetDirectory() {
			KD_CORE_ASSERT(GetActive());
			return GetActive()->m_Config.AbsAssetDirectory;
		}


		static const std::filesystem::path& GetProjectDirectory() {
			KD_CORE_ASSERT(GetActive());
			return GetActive()->m_ProjectDirectory;
		}


		// ProjectConfig::AbsAssetDirectory / path
		static std::filesystem::path GetAbsoluteAssetPath(const std::filesystem::path& path) {
			KD_CORE_ASSERT(GetActive());
			return  GetActive()->m_Config.AbsAssetDirectory / path;
		}
		// ProjectConfig::RelAssetDirectory / path
		static std::filesystem::path GetRelativeAssetPath(const std::filesystem::path& path) {
			KD_CORE_ASSERT(GetActive());
			return GetActive()->m_Config.RelAssetDirectory / path;
		}

		ProjectConfig& GetConfig() {
			return m_Config;
		}

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
		static Ref<Project> GetActive();



	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		friend class ProjectSerializer;

	};
}
