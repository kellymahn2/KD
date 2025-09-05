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
		std::unordered_map<std::string, const aiTexture*> m_IDToTexture;

		const aiMesh* GetMesh(uint32_t index);

		void ProcessNode(const aiNode* node, MeshTree& outTree);
		Ref<Mesh> ProcessMesh(const uint32_t* meshes, uint32_t meshCount);
		Ref<Material> ProcessMaterial(const aiMesh* mesh, const aiMaterial* material);

		void ProcessVertices(const aiMesh* mesh, MeshVertex* output);
		void ProcessSkinnedVertices(const aiMesh* mesh, SkinnedMeshVertex* output, uint32_t offset);

		uint64_t CalcVertexCount(const uint32_t* meshes, uint32_t meshCount);
		uint64_t CalcIndexCount(const uint32_t* meshes, uint32_t meshCount);

		template<typename T>
		uint64_t ProcessIndices(const aiMesh* mesh, T* indices)
		{
			uint64_t indexCount = 0;
			for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
				aiFace face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; ++j) {
					indices[indexCount++] = (T)face.mIndices[j];
				}
			}

			return indexCount;
		}
	};
}
