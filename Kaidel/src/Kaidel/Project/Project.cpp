#include "KDpch.h"
#include "Project.h"
#include "ProjectSerializer.h"
#include "Kaidel/Asset/AssetManager.h"
#include "Kaidel/Asset/AssetImporters/MetadataFile.h"
#include "Kaidel/Core/Application.h"

namespace Kaidel {
	static inline Ref<Project> s_ActiveProject;

	void Project::SaveAssets()
	{
		for (auto& [handle, asset] : AssetManager::BeginLoaded())
		{
			AssetManager::Serialize(AssetManager::GetAssetMetadata(handle).Path, asset);
		}
	}

	void Project::ConsumePathByFileWatcher(const Path& path, uint32_t amount)
	{
		if (!s_ActiveProject || s_ActiveProject->m_Watcher.GetWatchingPath().empty())
		{
			return;
		}
		s_ActiveProject->m_Watcher.Consume(path, amount);
	}

	Ref<Project> Project::New(const std::filesystem::path& path)
	{
		(s_ActiveProject = CreateRef<Project>());
		s_ActiveProject->m_ProjectDirectory = path;
		s_ActiveProject->m_Config.Name = "Untitled";

		//Make sure the default project path is empty.
		if (FileSystem::exists(s_ActiveProject->m_ProjectDirectory))
		{
			FileSystem::remove_all(s_ActiveProject->m_ProjectDirectory);

			FileSystem::create_directories(s_ActiveProject->m_ProjectDirectory);
		}

		s_ActiveProject->m_ProjectPath = s_ActiveProject->GetProjectFilePath(s_ActiveProject->m_ProjectDirectory);

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
					if (!FileSystem::exists(item.path().string() + ".mtd"))
					{
						continue;
					}
					
					AssetMetadata mtd;
					mtd.Path = item.path();
					

					MetadataFile file(mtd.Path, true);
					file.Resolve(mtd.Handle);
					mtd.Type = it->second;

					//Just register it.
					AssetManager::RegisterAsset(nullptr, mtd);
				}
			}
		}

		{
			FileSystem::recursive_directory_iterator rdi(path.parent_path());
			
			for (auto& item : rdi)
			{
				if (item.is_directory())
					continue;

				auto& extension = item.path().extension();

				const auto& supportedAssets = AssetManager::GetSupportedAssets();

				if (auto it = supportedAssets.find(extension.string()); it != supportedAssets.end())
				{
					AssetMetadata mtd;
					mtd.Path = item.path();
					mtd.Type = it->second;

					//Just register it.
					AssetManager::LoadAsset(mtd, mtd.Type);;
				}
			}
		}

		AssetManager::Wait();

		Application::Get().ExecuteMainThreadQueue();

		project->m_ProjectPath = path;

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

		s_ActiveProject->SaveAssets();
		
		return projectSerialized && assetsSerialized;
	}
	Ref<Project> Project::GetActive() {
		return s_ActiveProject;
	}


}
