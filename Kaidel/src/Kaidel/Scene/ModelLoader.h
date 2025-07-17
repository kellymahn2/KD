#pragma once

#include "Model.h"

namespace Kaidel {
	class ModelLoader {
	public:
		ModelLoader() = default;

		Ref<Model> Load(const Path& path);


	private:
		Ref<Model> m_Model;
		const aiScene* m_Scene;
		std::unordered_map<const aiMesh*, Ref<SkinTree>> m_MeshToSkin;

		void ProcessNode(const aiNode* node, MeshTree& outTree);
		Ref<Mesh> ProcessMesh(const aiMesh* mesh);
		Ref<Material> ProcessMaterial(const aiMesh* mesh, const aiMaterial* material);

		std::vector<MeshVertex> ProcessVertices(const aiMesh* mesh);
		std::vector<SkinnedMeshVertex> ProcessSkinnedVertices(const aiMesh* mesh);
		std::vector<uint16_t> ProcessIndices(const aiMesh* mesh);
	};
}
