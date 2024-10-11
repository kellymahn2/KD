#pragma once

struct MeshVertex
{
	glm::vec3 Position;
	glm::vec2 TexCoords;
	glm::vec3 ModelNormal;
	glm::vec3 ModelTangent;
	glm::vec3 ModelBitangent;
};



class Mesh : public IRCCounter<false> {
public:
	Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint16_t>& indices)
		:m_VertexCount(vertices.size()), m_IndexCount(indices.size())
	{
		m_VertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(MeshVertex));
		m_IndexBuffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint16_t), IndexType::Uint16);
	}

	Ref<VertexBuffer> GetVertexBuffer()const { return m_VertexBuffer; }
	Ref<IndexBuffer> GetIndexBuffer()const { return m_IndexBuffer; }
	Ref<Material> GetDefaultMaterial()const { return m_DefaultMaterial; }
	void SetDefaultMaterial(Ref<Material> mat) { m_DefaultMaterial = mat; }

	uint64_t GetVertexCount()const { return m_VertexCount; }
	uint64_t GetIndexCount()const { return m_IndexCount; }

private:
	Ref<VertexBuffer> m_VertexBuffer;
	Ref<IndexBuffer> m_IndexBuffer;
	Ref<Material> m_DefaultMaterial;
	uint64_t m_VertexCount, m_IndexCount;
};

struct MeshInstance {
	Ref<Kaidel::Mesh> Mesh;
	Ref<Material> OverrideMaterial;
};


class Model {
public:
	Model(const std::string& path)
		:m_ModelPath(path), m_ModelDir(m_ModelPath.parent_path())
	{
		LoadModel(path);
	}
	const auto& GetMeshes()const { return m_Meshes; }

private:
	void LoadModel(const std::string& path) {
		Assimp::Importer importer;
		const aiScene* scene =
			importer.ReadFile("assets/models/Sponza/Sponza.gltf",
				aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			KD_CORE_ASSERT(importer.GetErrorString());
		}

		ProcessNode(scene->mRootNode, scene);
	}
	void ProcessNode(const aiNode* node, const aiScene* scene) {
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene));
		}

		//process all the node children recursively
		for (uint32_t i = 0; i < node->mNumChildren; i++) {
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Ref<Mesh> ProcessMesh(const aiMesh* mesh, const aiScene* scene) {
		std::vector<MeshVertex> vertices;
		std::vector<uint16_t> indices;
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

			vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; ++j) {
				indices.push_back(face.mIndices[j]);
			}
		}

		Ref<Material> mat = ProcessMaterial(scene->mMaterials[mesh->mMaterialIndex], scene);

		Ref<Mesh> m = CreateRef<Mesh>(vertices, indices);

		m->SetDefaultMaterial(mat);

		return m;
	}

	Ref<Texture2D> GetMaterialTexture(aiTextureType type, Format format, const aiMaterial* material, const aiScene* scene) {
		if (!material->GetTextureCount(type))
			return {};

		aiString texturePath;
		KD_CORE_ASSERT(material->GetTexture(type, 0, &texturePath) == aiReturn_SUCCESS);
		KD_CORE_ASSERT(texturePath.length);

		std::string pathStr = texturePath.C_Str();

		return TextureLibrary::Load(m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, format);
	}

	glm::vec3 aiColor3DTovec3(const aiColor3D& v) {
		return { v.r,v.g,v.b };
	}

	template<typename T>
	T GetMaterialColor(const char* name, uint32_t type, uint32_t idx, const aiMaterial* material) {
		T value{};
		material->Get(name, type, idx, value);
		return value;
	}

	Ref<Material> ProcessMaterial(const aiMaterial* material, const aiScene* scene) {
		Ref<Material> mat = CreateRef<Material>(s_Data->DeferredPass.RenderPass);
		mat->SetTexture(MaterialTextureType::Albedo, GetMaterialTexture(aiTextureType_DIFFUSE, Format::RGBA8UN, material, scene));
		mat->SetTexture(MaterialTextureType::Mettalic, GetMaterialTexture(aiTextureType_UNKNOWN, Format::RGBA8UN, material, scene));
		mat->SetTexture(MaterialTextureType::Roughness, GetMaterialTexture(aiTextureType_UNKNOWN, Format::RGBA8UN, material, scene));
		mat->SetTexture(MaterialTextureType::Normal, GetMaterialTexture(aiTextureType_NORMALS, Format::RGBA8UN, material, scene));

		mat->SetAlbedoColor(aiColor3DTovec3(GetMaterialColor<aiColor3D>(AI_MATKEY_COLOR_AMBIENT, material)));
		mat->SetSpecular(aiColor3DTovec3(GetMaterialColor<aiColor3D>(AI_MATKEY_COLOR_AMBIENT, material)));
		return mat;
	}

private:
	Path m_ModelPath;
	Path m_ModelDir;
	std::vector<Ref<Mesh>> m_Meshes;
};
