#include "KDpch.h"
#include "ModelLoader.h"
#include "SkinResolver.h"
#include "AnimationResolver.h"
#include "Kaidel\Math\Math.h"
#include "Kaidel\Renderer\Renderer3D.h"

#include <glm/gtx/matrix_decompose.hpp>

#include <meshoptimizer.h>

namespace Kaidel {
#define MT
	static std::mutex s_MeshLoadingMutex;

	Ref<Model> ModelLoader::Load(const Path& path)
	{
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

		{
			SCOPED_TIMER("Assimp ReadFile");
			m_Scene =
				importer.ReadFile(path.string(),
					aiProcess_Triangulate | 
					aiProcess_OptimizeMeshes | 
					aiProcess_CalcTangentSpace | 
					aiProcess_FlipUVs |
					aiProcess_PopulateArmatureData |
					aiProcess_JoinIdenticalVertices |
					aiProcess_GenNormals |
					aiProcess_GenBoundingBoxes);
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

		for (uint32_t i = 0; i < m_Scene->mNumTextures; ++i)
		{
			const aiTexture* texture = m_Scene->mTextures[i];

			m_IDToTexture[texture->mFilename.C_Str()] = texture;
		}

		SCOPED_TIMER("Mesh loading");
		ProcessNode(m_Scene->mRootNode, *m_Model->m_Tree);

		JobSystem::GetMainJobSystem().Wait();
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
#ifdef MT
			JobSystem::GetMainJobSystem().Execute([&outTree, node, this]() {
				outTree.NodeMesh = ProcessMesh(node->mMeshes, node->mNumMeshes);
		});
#else
			outTree.NodeMesh = ProcessMesh(node->mMeshes, node->mNumMeshes);
#endif // MT
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			const aiNode* child = node->mChildren[i];
			outTree.Children.push_back(CreateScope<MeshTree>());
			ProcessNode(child, *outTree.Children.back());
		}
	}

	static AABB GetAABB(const aiMesh* mesh)
	{
		AABB result;
		result.Min = glm::vec3(mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z);
		result.Max = glm::vec3(mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z);
		return result;
	}

	Ref<Mesh> ModelLoader::ProcessMesh(const uint32_t* meshes, uint32_t meshCount)
	{

		bool skinned = GetMesh(meshes[0])->HasBones();

		uint64_t totalVertexCount = CalcVertexCount(meshes, meshCount);
		uint64_t totalIndexCount = CalcIndexCount(meshes, meshCount);

		uint64_t vertexSize = skinned ? sizeof(SkinnedMeshVertex) : sizeof(MeshVertex);

		void* vertices = nullptr;

		if (skinned)
			vertices = new SkinnedMeshVertex[totalVertexCount];
		else
			vertices = new MeshVertex[totalVertexCount];

		MeshInitializer initializer;

		initializer.IndexCount = totalIndexCount;
		initializer.VertexCount = totalVertexCount;
		initializer.VertexSize = vertexSize;
		initializer.IndexFormat = totalVertexCount > UINT16_MAX ? IndexType::Uint32 : IndexType::Uint16;
		initializer.Vertices = vertices;
		initializer.Indices = new uint8_t[totalIndexCount * (uint64_t)initializer.IndexFormat * 2];

		uint64_t vertexOffset = 0;
		uint64_t indexOffset = 0;


		for (uint32_t i = 0; i < meshCount; ++i)
		{
			const aiMesh* mesh = GetMesh(meshes[i]);

			if (skinned)
				ProcessSkinnedVertices(mesh, (SkinnedMeshVertex*)initializer.Vertices, vertexOffset);
			else
				ProcessVertices(mesh, (MeshVertex*)initializer.Vertices + vertexOffset);
			
			uint64_t vertexCount = mesh->mNumVertices;

			uint64_t indexCount = 0;

			void* startVertices = nullptr;

			if (skinned)
				startVertices = (SkinnedMeshVertex*)initializer.Vertices + vertexOffset;
			else
				startVertices = (MeshVertex*)initializer.Vertices + vertexOffset;

			if (initializer.IndexFormat == IndexType::Uint32)
			{
				indexCount = ProcessIndices<uint32_t>(mesh, (uint32_t*)initializer.Indices + indexOffset);
		
				uint32_t* startIndices = (uint32_t*)initializer.Indices + indexOffset;

				/*meshopt_optimizeVertexCache(startIndices, startIndices, indexCount, vertexCount);

				meshopt_optimizeOverdraw(startIndices, startIndices, indexCount,
					(const float*)startVertices, vertexCount, vertexSize, 1.05f);

				meshopt_optimizeVertexFetch(startVertices, startIndices, indexCount, startVertices, vertexCount, vertexSize);*/
				for (uint64_t i = 0; i < indexCount; ++i)
				{
					startIndices[i] += vertexOffset;
				}
			
			}
			else {
				indexCount = ProcessIndices<uint16_t>(mesh, (uint16_t*)initializer.Indices + indexOffset);

				uint16_t* startIndices = (uint16_t*)initializer.Indices + indexOffset;
				/*meshopt_optimizeVertexCache(startIndices, startIndices, indexCount, vertexCount);

				meshopt_optimizeOverdraw(startIndices, startIndices, indexCount,
					(const float*)startVertices, vertexCount, vertexSize, 1.05f);

				meshopt_optimizeVertexFetch(startVertices, startIndices, indexCount, startVertices, vertexCount, vertexSize);*/

				for (uint64_t i = 0; i < indexCount; ++i)
				{
					startIndices[i] += vertexOffset;
				}
			}

			Submesh submesh;
			submesh.VertexCount = vertexCount;
			submesh.VertexOffset = 0;
			submesh.IndexCount = indexCount;
			submesh.IndexOffset = indexOffset;
			submesh.BoundingBox = GetAABB(mesh);

			initializer.Submeshes.push_back(submesh);

			vertexOffset += vertexCount;
			indexOffset += indexCount;
		}

		std::unique_lock<std::mutex> lock(s_MeshLoadingMutex);

		for (uint32_t i = 0; i < meshCount; ++i)
		{
			const aiMesh* mesh = GetMesh(meshes[i]);

			Submesh& submesh = initializer.Submeshes[i];
			submesh.DefaultMaterial = ProcessMaterial(mesh, m_Scene->mMaterials[mesh->mMaterialIndex]);
		}

		Ref<Mesh> mesh;

		if (skinned)
			mesh = CreateRef<SkinnedMesh>(initializer, m_MeshToSkin[GetMesh(meshes[0])]);
		else
			mesh = CreateRef<Mesh>(initializer);

		delete[] initializer.Vertices;
		delete[] initializer.Indices;

		AABB boundingBox = mesh->GetSubmeshes()[0].BoundingBox;

		for (uint32_t i = 0; i < mesh->GetSubmeshes().size(); ++i)
		{
			auto& aabb = mesh->GetSubmeshes()[i].BoundingBox;
			boundingBox.Min = glm::min(aabb.Min, boundingBox.Min);
			boundingBox.Max = glm::max(aabb.Max, boundingBox.Max);
		}

		mesh->SetBoundingBox(boundingBox);
		return mesh;
	}

	Ref<StandardMaterialInstance> ModelLoader::ProcessMaterial(const aiMesh* mesh, const aiMaterial* material)
	{
		//SCOPED_TIMER("ProcessMaterial");
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

		Ref<Texture2D> albedo = {};
		if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			/*const aiTexture* texture = m_IDToTexture[texturePath.C_Str()];

			if (!texture->mHeight)
			{
				TextureLibrary::LoadFromBuffer(texture->pcData, texture->mWidth, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
			}*/

			std::string pathStr = texturePath.C_Str();

			albedo = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)StandardMaterialInstance::MaterialTextureType::Albedo;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Texture2D> metallic = {};
		if (material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			metallic = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA32F);

			{
				uint32_t type = (uint32_t)StandardMaterialInstance::MaterialTextureType::Metallic;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Texture2D> roughness = {};
		if (material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			roughness = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA32F);

			{
				uint32_t type = (uint32_t)StandardMaterialInstance::MaterialTextureType::Roughness;
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
				uint32_t type = (uint32_t)StandardMaterialInstance::MaterialTextureType::Normal;
				texturePaths.push_back({ type, pathStr });
			}
		}

		Ref<Texture2D> emmisive = {};
		if (material->GetTextureCount(aiTextureType_EMISSIVE))
		{
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			emmisive = TextureLibrary::Load(m_Model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)StandardMaterialInstance::MaterialTextureType::Emissive;
				texturePaths.push_back({ type, pathStr });
			}
		}

		glm::vec4 baseColor = glm::vec4(1.0f);

		aiColor3D col;
		if (material->Get(AI_MATKEY_BASE_COLOR, col) == aiReturn_SUCCESS)
		{
			baseColor = glm::vec4(col.r, col.g, col.b, 1.0);
		}

		Ref<StandardMaterialInstance> mat = CreateRef<StandardMaterialInstance>();
		mat->SetBaseColor(baseColor);
		mat->SetTexture(StandardMaterialInstance::MaterialTextureType::Albedo, albedo);
		mat->SetTexture(StandardMaterialInstance::MaterialTextureType::Metallic, metallic);
		mat->SetTexture(StandardMaterialInstance::MaterialTextureType::Roughness, roughness);
		mat->SetTexture(StandardMaterialInstance::MaterialTextureType::Normal, normals);
		mat->SetTexture(StandardMaterialInstance::MaterialTextureType::Emissive, emmisive);

		return mat;
	}

	void ModelLoader::ProcessVertices(const aiMesh* mesh, MeshVertex* output)
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

			output[i] = vertex;
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

	void ModelLoader::ProcessSkinnedVertices(const aiMesh* mesh, SkinnedMeshVertex* output, uint32_t offset)
	{
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

			output[i + offset] = vertex;
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

	uint64_t ModelLoader::CalcVertexCount(const uint32_t* meshes, uint32_t meshCount)
	{
		uint64_t vertexCount = 0;

		for (uint32_t i = 0; i < meshCount; ++i)
		{
			const aiMesh* mesh = GetMesh(meshes[i]);

			vertexCount += mesh->mNumVertices;
		}

		return vertexCount;
	}

	uint64_t ModelLoader::CalcIndexCount(const uint32_t* meshes, uint32_t meshCount)
	{
		uint64_t indexCount = 0;

		for (uint32_t i = 0; i < meshCount; ++i)
		{
			const aiMesh* mesh = GetMesh(meshes[i]);
			
			for (uint32_t j = 0; j < mesh->mNumFaces; ++j)
			{
				indexCount += mesh->mFaces[j].mNumIndices;
			}
		}

		return indexCount;
	}
}
