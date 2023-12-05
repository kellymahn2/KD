#include "KDpch.h"
#include "Project.h"
#include "ProjectSerializer.h"
namespace Kaidel {
	Ref<Project> Project::New()
	{
		(s_ActiveProject = CreateRef<Project>());
		s_ActiveProject->m_ProjectDirectory = "";
		return s_ActiveProject;
	}
	Ref<Project> Project::Load(const std::filesystem::path& path) {
		Ref<Project> project = CreateRef<Project>();
		ProjectSerializer serializer(project);
		if (!serializer.Deserialize(path)) {
			return {};
		}
		project->m_ProjectDirectory = path.parent_path();
		return s_ActiveProject = project;
	}
	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path)) {
			s_ActiveProject->m_ProjectDirectory = path;
			return true;
		}
		return false;
	}
}
