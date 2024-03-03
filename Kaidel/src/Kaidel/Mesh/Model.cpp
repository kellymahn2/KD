#include "KDpch.h"
#include "Model.h"
#include "Kaidel/Core/JobSystem.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>



extern int stbi__vertically_flip_on_load_global;

namespace Kaidel {




	namespace Utils {


		static void ProcessMesh(aiMesh* assimpMesh, const aiScene* scene, Asset<Mesh>& kdMesh, const std::unordered_map<uint32_t, Ref<Material>>& mats, aiMatrix4x4 parentMat) {
			std::vector<MeshVertex> vertices;
			std::vector<uint32_t> indices;
			
			aiVector3D center{ 0,0,0 };


			if (!assimpMesh->mNumVertices)
				return;

			for (uint32_t i = 0; i < assimpMesh->mNumVertices; ++i) {
				MeshVertex vertex{};
				vertex.Position = { assimpMesh->mVertices[i].x, assimpMesh->mVertices[i].y, assimpMesh->mVertices[i].z };

				center += assimpMesh->mVertices[i];

				if (assimpMesh->HasNormals())
					vertex.Normal = { assimpMesh->mNormals[i].x, assimpMesh->mNormals[i].y, assimpMesh->mNormals[i].z };

				if (assimpMesh->mTextureCoords[0]) {
					vertex.TexCoords = { assimpMesh->mTextureCoords[0][i].x, assimpMesh->mTextureCoords[0][i].y };
				}
				else {
					vertex.TexCoords = { 0, 0 };
				}

				vertices.push_back(vertex);
			}

			for (uint32_t i = 0; i < assimpMesh->mNumFaces; ++i) {
				aiFace face = assimpMesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

			Ref<Material> mat = {};

			auto it = mats.find(assimpMesh->mMaterialIndex);
			if (it != mats.end())
				mat = it->second;

			center /= static_cast<float>(assimpMesh->mNumVertices);

			center = parentMat * center;

			glm::vec3 c = { center.x,center.y,center.z };

			kdMesh = SingleAssetManager<Mesh>::Manage(CreateRef<Mesh>(assimpMesh->mName.C_Str(), vertices, indices, mat, c));
		}

		static uint64_t ProcessNode(aiNode* node, const aiScene* scene, std::vector<Asset<Mesh>>& meshIDs, const std::unordered_map<uint32_t, Ref<Material>>& mats, aiMatrix4x4 parentMat = aiMatrix4x4()) {
			if (!node) {
				return 0;
			}

			aiMatrix4x4 currentTransform = parentMat * node->mTransformation;

			// Process meshes at this node
			uint64_t nodeMeshCount = 0;
			for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				Asset<Mesh> kdMesh;
				ProcessMesh(mesh, scene, kdMesh, mats, currentTransform);
				nodeMeshCount++;
				meshIDs.emplace_back(kdMesh);
			}

			// Recursively process child nodes
			for (unsigned int i = 0; i < node->mNumChildren; ++i) {
				nodeMeshCount += ProcessNode(node->mChildren[i], scene, meshIDs, mats, currentTransform);
			}

			return nodeMeshCount;
		}
		static void ProcessTextures(const aiScene* scene, std::unordered_map<uint32_t, uint32_t>& embeddedTexturesByIndex, std::unordered_map<std::string, uint32_t>& embeddedTexturesByName,bool flipUVs) {
			for (uint32_t i = 0; i < scene->mNumTextures; ++i) {
				aiTexture* texture = scene->mTextures[i];
				/*for (uint32_t j = 0; j < texture->mWidth; ++j) {
					for (uint32_t k = 0; k < texture->mHeight; ++k) {
						std::swap(texture->pcData->r, texture->pcData->b);
					}
				}*/
				uint32_t textureIndex;
				//Compressed
				if (texture->mHeight == 0) {
					int x, y, channels;

					
					stbi_set_flip_vertically_on_load(flipUVs);

					uint8_t* decomp = stbi_load_from_memory((const uint8_t*)texture->pcData, texture->mWidth, &x, &y, &channels, 4);

					KD_CORE_ASSERT(decomp);
					textureIndex = MaterialTextureHandler::LoadTexture(decomp, x, y);
					
					
					stbi_image_free(decomp);
				}
				//Decompressed
				else
				{
					textureIndex = MaterialTextureHandler::LoadTexture(texture->pcData, texture->mWidth, texture->mHeight);
				}

				if (texture->mFilename.length > 0) {
					embeddedTexturesByName[texture->mFilename.C_Str()] = textureIndex;
				}
				embeddedTexturesByIndex[i] = textureIndex;
			}
		}

		static bool SetMaterialTexture(const aiMaterial* assimpMat, Ref<Material> kdMat, aiTextureType textureType, const aiScene* scene, const std::unordered_map<uint32_t, uint32_t>& embeddedTexturesByIndex, const std::unordered_map<std::string, uint32_t>& embeddedTexturesByName) {
			aiString path;
			if (assimpMat->GetTextureCount(textureType) == 0)
				return false;
			if (assimpMat->GetTexture(textureType, 0, &path) != aiReturn_SUCCESS)
				return false;

			if (path.length <= 0)
				return false;
			if (path.data[0] == '*') {
				uint32_t textureIndex = atoi(path.data + 1);
				auto it = embeddedTexturesByIndex.find(textureIndex);
				if (it != embeddedTexturesByIndex.end()) {

					switch (textureType)
					{
					case aiTextureType_DIFFUSE: kdMat->SetDiffuse(it->second); break;
					case aiTextureType_SPECULAR:kdMat->SetSpecular(it->second); break;	
					}
					return true;
				}
			
			}
			else {
				auto it = embeddedTexturesByName.find(path.C_Str());
				if (it != embeddedTexturesByName.end()) {

					switch (textureType)
					{
					case aiTextureType_DIFFUSE: kdMat->SetDiffuse(it->second); break;
					case aiTextureType_SPECULAR:kdMat->SetSpecular(it->second); break;
					}
					return true;
				}
			}

			return false;
		}

		static std::unordered_map<uint32_t, Ref<Material>> ProcessMaterials(const aiScene* scene, const std::unordered_map<uint32_t, uint32_t>& embeddedTexturesByIndex, const std::unordered_map<std::string, uint32_t>& embeddedTexturesByName) {
			std::unordered_map<uint32_t, Ref<Material>> result;
			for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
				aiMaterial* assimpMat = scene->mMaterials[i];
				Ref<Material> kdMat = CreateRef<Material>();
				bool diffuseLoaded = SetMaterialTexture(assimpMat, kdMat, aiTextureType_DIFFUSE, scene, embeddedTexturesByIndex, embeddedTexturesByName);
					bool specularLoaded = SetMaterialTexture(assimpMat, kdMat, aiTextureType_SPECULAR, scene, embeddedTexturesByIndex, embeddedTexturesByName);
				if (diffuseLoaded || specularLoaded)
					result[i] = kdMat;
			}
			return result;
		}
	}

	
	Ref<Model> Model::Load(const std::filesystem::path& modelPath, bool flipUVs) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelPath.string(), aiProcess_Triangulate  | aiProcess_EmbedTextures | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes|aiProcess_PreTransformVertices);
		KD_ASSERT(scene && ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) == 0) && scene->mRootNode, "Couldn't load model at path {} with error {}",modelPath.string(),importer.GetErrorString());
		Ref<Model> model = CreateRef<Model>();
		model->m_ModelPath = modelPath;
		std::unordered_map<uint32_t, uint32_t> embeddedTexturesByIndex;
		std::unordered_map<std::string, uint32_t> embeddedTexturesByName;
		{
			Timer timer("Proccessing Textures");
			Utils::ProcessTextures(scene, embeddedTexturesByIndex,embeddedTexturesByName,flipUVs);
		}
		std::unordered_map<uint32_t, Ref<Material>> mats;
		{
			Timer timer("Material Loading");
			mats = Utils::ProcessMaterials(scene, embeddedTexturesByIndex,embeddedTexturesByName);
		}
			
		model->m_MeshCount = Utils::ProcessNode(scene->mRootNode, scene, model->m_MeshHandles,mats);
		
		importer.FreeScene();

		return model;
	}



	void Model::Draw(const glm::mat4& transform, Ref<Material>& mat, const Math::Frustum& frustum) {
			
	}
	void Model::Flush() {
		/*for (uint32_t i = 0; i < m_Meshes.size(); ++i)
			m_Meshes[i].Flush();*/
	}
}
