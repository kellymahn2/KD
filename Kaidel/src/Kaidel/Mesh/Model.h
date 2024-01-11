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
		void Draw(const glm::mat4& transform,Ref<Material>& mat);
		void Flush();
	private:
		void ProcessNode(aiNode* node, const aiScene* scene);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene,Mesh& m);
		std::vector<Mesh> m_Meshes;
	};

}
