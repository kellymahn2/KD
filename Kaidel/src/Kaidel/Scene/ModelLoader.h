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
		std::unordered_map<const aiMesh*, Ref<Skin>> m_MeshToSkin;
		std::unordered_map<const aiNode*, Ref<AnimationTree>> m_NodeToAnim;
		
		const aiMesh* GetMesh(uint32_t index);

		void ProcessNode(const aiNode* node, MeshTree& outTree);
		Ref<Mesh> ProcessMesh(const uint32_t* meshes, uint32_t meshCount);
		Ref<Material> ProcessMaterial(const aiMesh* mesh, const aiMaterial* material);

		void ProcessVertices(const aiMesh* mesh, std::vector<MeshVertex>& output);
		void ProcessSkinnedVertices(const aiMesh* mesh, std::vector<SkinnedMeshVertex>& output);
		void ProcessIndices(const aiMesh* mesh, uint32_t offset, std::vector<uint16_t>& indices);
	};
}
