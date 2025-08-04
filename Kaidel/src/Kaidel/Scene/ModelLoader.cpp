#include "KDpch.h"
#include "ModelLoader.h"
#include "SkinResolver.h"
#include "AnimationResolver.h"
#include "Kaidel\Math\Math.h"

#include <glm/gtx/matrix_decompose.hpp>

namespace Kaidel {

	Ref<Model> ModelLoader::Load(const Path& path)
	{
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		{
			
			//SCOPED_TIMER("Assimp ReadFile");
			m_Scene =
				importer.ReadFile(path.string(),
					aiProcess_Triangulate | 
					aiProcess_OptimizeMeshes | 
					aiProcess_CalcTangentSpace | 
					aiProcess_FlipUVs |
					aiProcess_GlobalScale);
		}
		if (!m_Scene || m_Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_Scene->mRootNode)
		{
			KD_CORE_ASSERT(importer.GetErrorString());
			return {};
		}

		m_Model = CreateRef<Model>();
		m_Model->m_ModelDir = path.parent_path();
		m_Model->m_ModelPath = path;
		m_Model->m_Tree = CreateScope<MeshTree>();

		m_MeshToSkin = SkinResolver(m_Scene).ResolveSkinTrees();

		m_NodeToAnim = AnimationResolver(m_Scene).ResolveAnimations();

		ProcessNode(m_Scene->mRootNode, *m_Model->m_Tree);
		return m_Model;
	}
	
	const aiMesh* ModelLoader::GetMesh(uint32_t index)
	{
		return m_Scene->mMeshes[index];
	}

	void ModelLoader::ProcessNode(const aiNode* node, MeshTree& outTree)
	{
		outTree.NodeName = node->mName.C_Str();

		glm::mat4 nodeTransform = glm::transpose(*(const glm::mat4*)&node->mTransformation);

		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(nodeTransform, outTree.Scale, outTree.Rotation, outTree.Position, skew, perspective);

		if (auto it = m_NodeToAnim.find(node); it != m_NodeToAnim.end())
		{
			outTree.NodeAnimation = it->second;
		}

		if(node->mNumMeshes){
			outTree.NodeMesh = ProcessMesh(node->mMeshes, node->mNumMeshes);
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			const aiNode* child = node->mChildren[i];
			outTree.Children.push_back(CreateScope<MeshTree>());
			ProcessNode(child, *outTree.Children.back());
		}
	}

	Ref<Mesh> ModelLoader::ProcessMesh(const uint32_t* meshes, uint32_t meshCount)
	{
		if (GetMesh(meshes[0])->HasBones())
		{
			std::vector<SkinnedMeshVertex> vertices;
			std::vector<uint16_t> indices;
			std::vector<Submesh> submeshes;

			Ref<Skin> skin = m_MeshToSkin[GetMesh(meshes[0])];

			for (uint32_t i = 0; i < meshCount; ++i)
			{
				const aiMesh* mesh = GetMesh(meshes[i]);
				
				KD_CORE_ASSERT(mesh->HasBones(), "All submeshes should have bones.");
				KD_CORE_ASSERT(m_MeshToSkin[mesh] == skin, "All submeshes should use the same skin");
				
				uint32_t vertexOffset = vertices.size();
				ProcessSkinnedVertices(mesh, vertices);
				uint32_t vertexCount = vertices.size() - vertexOffset;

				uint32_t indexOffset = indices.size();
				ProcessIndices(mesh, vertexOffset, indices);
				uint32_t indexCount = indices.size() - indexOffset;
				
				Submesh submesh;
				submesh.VertexCount = vertexCount;
				submesh.VertexOffset = vertexOffset;
				submesh.IndexCount = indexCount;
				submesh.IndexOffset = indexOffset;
				submesh.DefaultMaterial = ProcessMaterial(mesh, m_Scene->mMaterials[mesh->mMaterialIndex]);

				submeshes.push_back(submesh);
			}

			return CreateRef<SkinnedMesh>(vertices, indices, submeshes, skin);
		}
		else
		{
			std::vector<MeshVertex> vertices;
			std::vector<uint16_t> indices;
			std::vector<Submesh> submeshes;

			for (uint32_t i = 0; i < meshCount; ++i)
			{
				const aiMesh* mesh = GetMesh(meshes[i]);

				uint32_t vertexOffset = vertices.size();
				ProcessVertices(mesh, vertices);
				uint32_t vertexCount = vertices.size() - vertexOffset;

				uint32_t indexOffset = indices.size();
				ProcessIndices(mesh, vertexOffset, indices);
				uint32_t indexCount = indices.size() - indexOffset;

				Submesh submesh;
				submesh.VertexCount = vertexCount;
				submesh.VertexOffset = vertexOffset;
				submesh.IndexCount = indexCount;
				submesh.IndexOffset = indexOffset;
				submesh.DefaultMaterial = ProcessMaterial(mesh, m_Scene->mMaterials[mesh->mMaterialIndex]);

				submeshes.push_back(submesh);
			}

			return CreateRef<Mesh>(vertices, indices, submeshes);
		}
	}

	Ref<Material> ModelLoader::ProcessMaterial(const aiMesh* mesh, const aiMaterial* material)
	{
		//SCOPED_TIMER("ProcessMaterial");
		Ref<Texture2D> albedo = {};
		std::vector<std::pair<uint32_t, std::string>> texturePaths;

		static const char* names[] =
		{
			"aiTextureType_NONE",
			"aiTextureType_DIFFUSE",
			"aiTextureType_SPECULAR",
			"aiTextureType_AMBIENT",
			"aiTextureType_EMISSIVE",
			"aiTextureType_HEIGHT",
			"aiTextureType_NORMALS",
			"aiTextureType_SHININESS",
			"aiTextureType_OPACITY",
			"aiTextureType_DISPLACEMENT",
			"aiTextureType_LIGHTMAP",
			"aiTextureType_REFLECTION",
			"aiTextureType_BASE_COLOR",
			"aiTextureType_NORMAL_CAMERA",
			"aiTextureType_EMISSION_COLOR",
			"aiTextureType_METALNESS",
			"aiTextureType_DIFFUSE_ROUGHNESS",
			"aiTextureType_AMBIENT_OCCLUSION",
			"aiTextureType_UNKNOWN",
			"aiTextureType_SHEEN",
			"aiTextureType_CLEARCOAT",
			"aiTextureType_TRANSMISSION",
			"aiTextureType_MAYA_BASE",
			"aiTextureType_MAYA_SPECULAR",
			"aiTextureType_MAYA_SPECULAR_COLOR",
			"aiTextureType_MAYA_SPECULAR_ROUGHNESS",
			"aiTextureType_ANISOTROPY",
			"aiTextureType_GLTF_METALLIC_ROUGHNESS"
		};


		for (uint32_t i = 0; i <= aiTextureType_GLTF_METALLIC_ROUGHNESS; ++i)
		{
			if (material->GetTextureCount((aiTextureType)i))
			{
				aiString texturePath;
				KD_CORE_ASSERT(material->GetTexture((aiTextureType)i, 0, &texturePath) == aiReturn_SUCCESS);
				KD_CORE_ASSERT(texturePath.length);
				KD_INFO(
					"material {}, on mesh {}, has {} texture at {}", 
					material->GetName().C_Str(), mesh->mName.C_Str(), names[i], texturePath.C_Str());
			}
		}

		if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			albedo = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Albedo;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Texture2D> metallic = {};
		if (material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			metallic = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Mettalic;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Texture2D> roughness = {};
		if (material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			roughness = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Roughness;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Texture2D> normals = {};
		if (material->GetTextureCount(aiTextureType_NORMALS)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			normals = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Normal;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Material> mat = CreateRef<Material>(GetDeferredPassRenderPass());
		mat->SetTexture(MaterialTextureType::Albedo, albedo);
		mat->SetTexture(MaterialTextureType::Mettalic, metallic);
		mat->SetTexture(MaterialTextureType::Roughness, roughness);
		mat->SetTexture(MaterialTextureType::Normal, normals);
		return mat;
	}

	void ModelLoader::ProcessVertices(const aiMesh* mesh, std::vector<MeshVertex>& output)
	{

		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			MeshVertex vertex{};
			glm::vec3 vector{};

			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.ModelTangent = vector;

			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.ModelBitangent = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.ModelNormal = vector;

			if (mesh->HasTextureCoords(0)) {
				glm::vec2 vec{};
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else {
				vertex.TexCoords = glm::vec2(0, 0);
			}

			output.push_back(vertex);
		}
	}

	static SkinTree* FindBoneInTree(const aiBone* bone, SkinTree* tree)
	{
		if (tree->Name == bone->mName.C_Str())
			return tree;

		for (auto& child : tree->Children)
		{
			SkinTree* ptr = FindBoneInTree(bone, &child);

			if (ptr)
				return ptr;
		}

		return nullptr;
	}

	void ModelLoader::ProcessSkinnedVertices(const aiMesh* mesh, std::vector<SkinnedMeshVertex>& output)
	{
		uint32_t offset = output.size();
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			SkinnedMeshVertex vertex{};
			glm::vec3 vector{};

			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.ModelTangent = vector;

			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.ModelBitangent = vector;

			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.ModelNormal = vector;

			if (mesh->HasTextureCoords(0)) {
				glm::vec2 vec{};
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else {
				vertex.TexCoords = glm::vec2(0, 0);
			}

			output.push_back(vertex);
		}

		Ref<Skin> skin = m_MeshToSkin[mesh];
		KD_CORE_ASSERT(skin);

		std::unordered_map<const aiBone*, SkinTree*> boneToSkin;

		for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
			const aiBone* bone = mesh->mBones[i];

			boneToSkin[bone] = FindBoneInTree(bone, &skin->Tree);
		}

		for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
			const aiBone* bone = mesh->mBones[i];

			for (uint32_t j = 0; j < bone->mNumWeights; ++j) {
				const aiVertexWeight& weight = bone->mWeights[j];

				if (weight.mWeight == 0.0f)
					continue;
				
				SkinnedMeshVertex& vertex = output[weight.mVertexId + offset];

				uint32_t k = 0;
				for (; k < MaxBoneCount; ++k)
				{
					if (vertex.BoneWeight[k] == 0.0f && vertex.BoneID[k] == -1)
					{
						break;
					}
				}
				
				if (k < MaxBoneCount)
				{
					vertex.BoneWeight[k] = weight.mWeight;
					vertex.BoneID[k] = boneToSkin[bone]->ID;
				}
			}
		}
	}

	void ModelLoader::ProcessIndices(const aiMesh* mesh, uint32_t offset, std::vector<uint16_t>& indices)
	{
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; ++j) {
				indices.push_back(face.mIndices[j] + offset);
			}
		}
	}

}
