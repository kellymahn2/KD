#include "KDpch.h"
#include "Model.h"
#include "Kaidel/Core/JobSystem.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
namespace Kaidel {


	void Model::ProcessNode(aiNode* node, const aiScene* scene) {
		uint32_t s = m_Meshes.size();
		m_Meshes.resize(node->mNumMeshes+s);
		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh,scene,m_Meshes[s+i]);
		}
		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			ProcessNode(node->mChildren[i], scene);
		}
	}
	void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene,Mesh& m) {
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
		m.m_Vertices = vertices;
		m.m_IndexCount = indices.size();
		m.Setup(indices);
	}
	Ref<Model> Model::Load(const std::filesystem::path& modelPath) {


		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);
		KD_ASSERT(scene && ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) == 0) && scene->mRootNode, "Couldn't load model at path {} with error {}",modelPath.string(),importer.GetErrorString());
		Ref<Model> model = CreateRef<Model>();
		model->ProcessNode(scene->mRootNode, scene);
		return model;
	}

	void Model::Draw(const glm::mat4& transform, Ref<Material>& mat, const Math::Frustum& frustum) {
		
		//Timer timer("Total");
		{
			//Timer timer("Dispach");
			JobSystem::GetMainJobSystem().Dispatch(m_Meshes.size(), 16, [&](JobDispatchArgs& args) {
				if(frustum.IsCulled(m_Meshes[args.jobIndex].GetBoundingBox().Transform(transform)))
					return;
				m_Meshes[args.jobIndex].Draw(transform, mat);
				});
		}

		{
			//Timer timer("Wait");
			JobSystem::GetMainJobSystem().Wait();
		}
		/*for (uint32_t i = 0; i < m_Meshes.size(); ++i) {
			m_Meshes[i].Draw(transform, mat);
		}*/
	}
	void Model::Flush() {
		for (uint32_t i = 0; i < m_Meshes.size(); ++i)
			m_Meshes[i].Flush();
	}
}
