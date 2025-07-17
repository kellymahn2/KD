#include "KDpch.h"
#include "ModelLoader.h"
#include "SkinResolver.h"
#include "Kaidel\Math\Math.h"

namespace Kaidel {

	Ref<Model> ModelLoader::Load(const Path& path)
	{
		Assimp::Importer importer;

		{
			SCOPED_TIMER("Assimp ReadFile");
			m_Scene =
				importer.ReadFile(path.string(),
					aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GlobalScale);
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

		ProcessNode(m_Scene->mRootNode, *m_Model->m_Tree);
		return m_Model;
	}

	void ModelLoader::ProcessNode(const aiNode* node, MeshTree& outTree)
	{
		outTree.NodeName = node->mName.C_Str();

		glm::mat4 nodeTransform = glm::transpose((const glm::mat4&)node->mTransformation);

		Math::DecomposeTransform(nodeTransform, outTree.Position, outTree.Rotation, outTree.Scale);

		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			const aiMesh* mesh = m_Scene->mMeshes[node->mMeshes[i]];
			outTree.NodeMesh = ProcessMesh(mesh);
			//Take only the first mesh in node (usually only one mesh exists in each node, if any.)
			break;
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			const aiNode* child = node->mChildren[i];
			outTree.Children.push_back(CreateScope<MeshTree>());
			ProcessNode(child, *outTree.Children.back());
		}
	}
	Kaidel::Ref<Kaidel::Mesh> ModelLoader::ProcessMesh(const aiMesh* mesh)
	{
		SCOPED_TIMER("ProcessMesh");

		if (mesh->HasBones())
		{
			Ref<SkinnedMesh> m = CreateRef<SkinnedMesh>(ProcessSkinnedVertices(mesh), ProcessIndices(mesh), m_MeshToSkin[mesh]);
			m->SetDefaultMaterial(ProcessMaterial(mesh, m_Scene->mMaterials[mesh->mMaterialIndex]));
			return m;
		}

		Ref<Mesh> m = CreateRef<Mesh>(ProcessVertices(mesh), ProcessIndices(mesh));
		m->SetDefaultMaterial(ProcessMaterial(mesh, m_Scene->mMaterials[mesh->mMaterialIndex]));
		return m;
	}
	Kaidel::Ref<Kaidel::Material> ModelLoader::ProcessMaterial(const aiMesh* mesh, const aiMaterial* material)
	{
		SCOPED_TIMER("ProcessMaterial");
		Ref<Texture2D> albedo = {};
		std::vector<std::pair<uint32_t, std::string>> texturePaths;
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
		if (material->GetTextureCount(aiTextureType_UNKNOWN)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			metallic = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Mettalic;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Texture2D> roughness = {};
		if (material->GetTextureCount(aiTextureType_UNKNOWN)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == aiReturn_SUCCESS);
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

	std::vector<MeshVertex> ModelLoader::ProcessVertices(const aiMesh* mesh)
	{
		std::vector<MeshVertex> vertices(mesh->mNumVertices, MeshVertex());

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

			vertices[i] = vertex;
		}

		return vertices;
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

	std::vector<SkinnedMeshVertex> ModelLoader::ProcessSkinnedVertices(const aiMesh* mesh)
	{
		std::vector<SkinnedMeshVertex> vertices(mesh->mNumVertices, SkinnedMeshVertex());

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

			vertices[i] = vertex;
		}

		Ref<SkinTree> tree = m_MeshToSkin[mesh];
		KD_CORE_ASSERT(tree);

		std::unordered_map<const aiBone*, SkinTree*> boneToSkin;

		for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
			const aiBone* bone = mesh->mBones[i];

			boneToSkin[bone] = FindBoneInTree(bone, tree.Get());
		}

		for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
			const aiBone* bone = mesh->mBones[i];

			for (uint32_t j = 0; j < bone->mNumWeights; ++j) {
				const aiVertexWeight& weight = bone->mWeights[j];

				if (weight.mWeight == 0.0f)
					continue;
				
				SkinnedMeshVertex& vertex = vertices[weight.mVertexId];

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

		return vertices;
	}

	std::vector<uint16_t> ModelLoader::ProcessIndices(const aiMesh* mesh)
	{
		std::vector<uint16_t> indices;

		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; ++j) {
				indices.push_back(face.mIndices[j]);
			}
		}

		return indices;
	}

}
