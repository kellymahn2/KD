#pragma once
#include "Mesh.h"
#include <filesystem>
struct aiNode;
struct aiMesh;
struct aiScene;
namespace Kaidel {
	class Model {
	public:
		static Ref<Model> Load(const std::filesystem::path& modelPath);
		void Draw();
	private:
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

		std::vector<Mesh> m_Meshes;
	};
}
