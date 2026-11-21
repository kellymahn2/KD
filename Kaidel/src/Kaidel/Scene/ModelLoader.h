#pragma once

#include "Model.h"
#include "Kaidel/Asset/AssetImporters/MetadataFile.h"

namespace Kaidel {

	struct ModelDependencies 
	{
		std::unordered_set<Path> Textures;
	};

	class ModelLoader{
	public:
		ModelLoader(const Path& path = {});

		~ModelLoader();

		const ModelDependencies& GetDependencies(const Path& path);
		const ModelDependencies& GetDependencies() const;

		void LoadInto(Ref<Model> model, const AssetMetadata& metadata, MetadataFile* mtdFile);

	private:

		void OpenScene(const Path& path);

		void NodeDFS(const aiNode* currentNode, uint32_t& index);

		void GenerateMetadata();

		const aiMesh* GetMesh(uint32_t index);

		void ProcessNode(const aiNode* node, MeshTree& outTree);
		Ref<Mesh> ProcessMesh(const uint32_t* meshes, uint32_t meshCount);
		Ref<StandardMaterial> ProcessMaterial(const aiMesh* mesh, const aiMaterial* material);

		void ProcessVertices(const aiMesh* mesh, MeshVertex* output);
		void ProcessSkinnedVertices(const aiMesh* mesh, SkinnedMeshVertex* output, uint32_t offset);

		uint64_t CalcVertexCount(const uint32_t* meshes, uint32_t meshCount);
		uint64_t CalcIndexCount(const uint32_t* meshes, uint32_t meshCount);

		void Resolve(UUID& handle)
		{
			m_MtdFile->Resolve(handle, m_CurrentAssetIndex++);
		}

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
	private:
		Ref<Model> m_Model;
		const aiScene* m_Scene = nullptr;
		const AssetMetadata* m_Metadata;
		std::unordered_map<const void*, uint32_t> m_ResourceToID;
		//std::unordered_map<uint32_t, const aiNode*> m_IDToNode;
		std::unordered_map<const aiMesh*, Ref<Skin>> m_MeshToSkin;
		std::unordered_map<const aiNode*, Ref<AnimationTree>> m_NodeToAnim;
		std::unordered_map<std::string, const aiTexture*> m_IDToTexture;

		ModelDependencies m_Dependencies;

		MetadataFile* m_MtdFile;

		bool m_HasMtd = false;

		uint64_t m_CurrentAssetIndex = 1;
	};
}
