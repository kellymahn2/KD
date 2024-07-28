#include "KDpch.h"
#include "Project.h"
#include "ProjectSerializer.h"
namespace Kaidel {
	static inline Ref<Project> s_ActiveProject;
	Ref<Project> Project::New()
	{
		(s_ActiveProject = CreateRef<Project>());
		s_ActiveProject->m_ProjectDirectory = "";
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
		return s_ActiveProject = project;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		bool projectSerialized = false;
		bool assetsSerialized = false;
		{
			ProjectSerializer serializer(s_ActiveProject);
			if (serializer.Serialize(path / s_ActiveProject->m_Config.Name)) {
				s_ActiveProject->m_ProjectDirectory = path;
				projectSerialized = true;
			}
		}
		
		return projectSerialized;
	}
	Ref<Project> Project::GetActive() {
		return s_ActiveProject;
	}


}
