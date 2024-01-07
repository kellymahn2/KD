#include "KDpch.h"
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
namespace Kaidel {


	void Model::ProcessNode(aiNode* node, const aiScene* scene) {
		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh,scene));
		}
		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			ProcessNode(node->mChildren[i], scene);
		}
	}
	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
		std::vector<MeshVertex> vertices;
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			MeshVertex vertex{};
			vertex.Position = { mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z };
			if(mesh->HasNormals())
				vertex.Normal = { mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z };

			if (mesh->mTextureCoords[0]) {

				vertex.TexCoords = { mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y };
			}
			else {
				vertex.TexCoords = { 0,0 };
			}
			vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		Ref<Material> mat = CreateRef<Material>();
		return Mesh(vertices, indices, mat);
	}
	Ref<Model> Model::Load(const std::filesystem::path& modelPath) {


		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);
		KD_ASSERT(scene && ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) == 0) && scene->mRootNode, "Couldn't load model at path {} with error {}",modelPath.string(),importer.GetErrorString());
		Ref<Model> model = CreateRef<Model>();
		model->ProcessNode(scene->mRootNode, scene);
		return model;
	}

	void Model::Draw() {
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
			m_Meshes[i].Draw();
	}

}
