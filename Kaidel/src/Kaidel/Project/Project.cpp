#include "KDpch.h"
#include "Project.h"
#include "ProjectSerializer.h"
#include "Kaidel/Asset/AssetManager.h"

namespace Kaidel {
	static inline Ref<Project> s_ActiveProject;
	Ref<Project> Project::New()
	{
		(s_ActiveProject = CreateRef<Project>());
		s_ActiveProject->m_ProjectDirectory = "SandboxProject";

		//Make sure the default project path is empty.
		if (FileSystem::exists(s_ActiveProject->m_ProjectDirectory))
		{
			FileSystem::remove_all(s_ActiveProject->m_ProjectDirectory);

			FileSystem::create_directories(s_ActiveProject->m_ProjectDirectory);
		}

		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path) {
		Ref<Project> project = CreateRef<Project>();
		{
			ProjectSerializer serializer(project);
			if (!serializer.Deserialize(path)) {
				return {};
			}
		}

		{
			FileSystem::recursive_directory_iterator rdi(path.parent_path());

			for (auto& item : rdi)
			{
				if(item.is_directory())
					continue;

				auto& extension = item.path().extension();

				const auto& supportedAssets = AssetManager::GetSupportedAssets();

				if (auto it = supportedAssets.find(extension.string()); it != supportedAssets.end())
				{
					KD_INFO("{}", item.path().string());

					AssetMetadata mtd;
					mtd.Path = item.path();

					AssetManager::LoadAsset(mtd, it->second);
				}
			}
		}

		AssetManager::Wait();

		return s_ActiveProject = project;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		bool projectSerialized = false;
		bool assetsSerialized = false;

		FileSystem::path projectFilePath = s_ActiveProject->GetProjectFilePath(path);

		{
			ProjectSerializer serializer(s_ActiveProject);
			if (serializer.Serialize(projectFilePath)) {
				s_ActiveProject->m_ProjectDirectory = path;
				projectSerialized = true;
			}
		}

		//Serialize assets.
		{
			//AssetManager::Serialize(path / ("AssetMap.amap"));
			for (auto& [handle, asset] : AssetManager::BeginLoaded())
			{
				AssetManager::Serialize(AssetManager::GetAssetMetadata(handle).Path, asset);
			}

			assetsSerialized = true;
		}
		
		return projectSerialized && assetsSerialized;
	}
	Ref<Project> Project::GetActive() {
		return s_ActiveProject;
	}


}
