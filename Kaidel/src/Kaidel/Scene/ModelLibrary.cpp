#include "KDpch.h"
#include "ModelLibrary.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Kaidel {

	static struct ModelLibraryData {
		std::unordered_map<Path, Ref<Model>> LoadedModels;
		Ref<Model> Cube;
		Ref<Model> Sphere;
		Ref<Model> Cylinder;
	}* s_LibraryData;

	void ModelLibrary::Init()
	{
		s_LibraryData = new ModelLibraryData;

		//Cube
		{
			s_LibraryData->Cube = CreateRef<Model>();

			const std::vector<MeshVertex> vertices = {
				// Front face
				{glm::vec3(-0.5f, -0.5f,  0.5f),glm::vec2(0,0),glm::vec3(0,0,1),glm::vec3(1,0,0),glm::vec3(0,1,0)},
				{glm::vec3(0.5f, -0.5f,  0.5f),glm::vec2(1,0),glm::vec3(0,0,1),glm::vec3(1,0,0),glm::vec3(0,1,0)},
				{glm::vec3(0.5f,  0.5f,  0.5f),glm::vec2(1,1),glm::vec3(0,0,1),glm::vec3(1,0,0),glm::vec3(0,1,0)},
				{glm::vec3(-0.5f,  0.5f,  0.5f),glm::vec2(0,1),glm::vec3(0,0,1),glm::vec3(1,0,0),glm::vec3(0,1,0)},
				// Back face
				{glm::vec3(0.5f, -0.5f, -0.5f),glm::vec2(0,0),glm::vec3(0,0,-1),glm::vec3(-1,0,0),glm::vec3(0,1,0)},
				{glm::vec3(-0.5f, -0.5f, -0.5f),glm::vec2(1,0),glm::vec3(0,0,-1),glm::vec3(-1,0,0),glm::vec3(0,1,0)},
				{glm::vec3(-0.5f,  0.5f, -0.5f),glm::vec2(1,1),glm::vec3(0,0,-1),glm::vec3(-1,0,0),glm::vec3(0,1,0)},
				{glm::vec3(0.5f,  0.5f, -0.5f),glm::vec2(0,1),glm::vec3(0,0,-1),glm::vec3(-1,0,0),glm::vec3(0,1,0)},
				// Left face
				{glm::vec3(-0.5f, -0.5f,  -0.5f),glm::vec2(0,0),glm::vec3(-1,0,0),glm::vec3(0,0,1),glm::vec3(0,1,0)},
				{glm::vec3(-0.5f, -0.5f, 0.5f),glm::vec2(1,0),glm::vec3(-1,0,0),glm::vec3(0,0,1),glm::vec3(0,1,0)},
				{glm::vec3(-0.5f,  0.5f, 0.5f),glm::vec2(1,1),glm::vec3(-1,0,0),glm::vec3(0,0,1),glm::vec3(0,1,0)},
				{glm::vec3(-0.5f,  0.5f,  -0.5f),glm::vec2(0,1),glm::vec3(-1,0,0),glm::vec3(0,0,1),glm::vec3(0,1,0)},
				// Right face
				{glm::vec3(0.5f, -0.5f,  0.5f),glm::vec2(0,0),glm::vec3(1,0,0),glm::vec3(0,0,-1),glm::vec3(0,1,0)},
				{glm::vec3(0.5f, -0.5f, -0.5f),glm::vec2(1,0),glm::vec3(1,0,0),glm::vec3(0,0,-1),glm::vec3(0,1,0)},
				{glm::vec3(0.5f,  0.5f, -0.5f),glm::vec2(1,1),glm::vec3(1,0,0),glm::vec3(0,0,-1),glm::vec3(0,1,0)},
				{glm::vec3(0.5f,  0.5f,  0.5f),glm::vec2(0,1),glm::vec3(1,0,0),glm::vec3(0,0,-1),glm::vec3(0,1,0)},
				// Top face
				{glm::vec3(-0.5f,  0.5f,  0.5f),glm::vec2(0,0),glm::vec3(0,1,0),glm::vec3(1,0,0),glm::vec3(0,0,-1)},
				{glm::vec3(0.5f,  0.5f,  0.5f),glm::vec2(1,0),glm::vec3(0,1,0),glm::vec3(1,0,0),glm::vec3(0,0,-1)},
				{glm::vec3(0.5f,  0.5f, -0.5f),glm::vec2(1,1),glm::vec3(0,1,0),glm::vec3(1,0,0),glm::vec3(0,0,-1)},
				{glm::vec3(-0.5f,  0.5f, -0.5f),glm::vec2(0,1),glm::vec3(0,1,0),glm::vec3(1,0,0),glm::vec3(0,0,-1)},
				// Bottom face
				{glm::vec3(-0.5f, -0.5f,  -0.5f),glm::vec2(0,0),glm::vec3(0,-1,0),glm::vec3(1,0,0),glm::vec3(0,0,1)},
				{glm::vec3(0.5f, -0.5f,  -0.5f),glm::vec2(1,0),glm::vec3(0,-1,0),glm::vec3(1,0,0),glm::vec3(0,0,1)},
				{glm::vec3(0.5f, -0.5f, 0.5f),glm::vec2(1,1),glm::vec3(0,-1,0),glm::vec3(1,0,0),glm::vec3(0,0,1)},
				{glm::vec3(-0.5f, -0.5f, 0.5f),glm::vec2(0,1),glm::vec3(0,-1,0),glm::vec3(1,0,0),glm::vec3(0,0,1)},
			};
			const std::vector<unsigned short> indices = {
				// Front face
				0, 1, 2,
				2, 3, 0,
				// Back face
				4, 5, 6,
				6, 7, 4,
				// Left face
				8, 9, 10,
				10, 11, 8,
				// Right face
				12, 13, 14,
				14, 15, 12,
				// Top face
				16, 17, 18,
				18, 19, 16,
				// Bottom face
				20, 21, 22,
				22, 23, 20,
			};

			Ref<Material> mat = CreateRef<Material>(GetDeferredPassRenderPass());
			mat->SetTexture(MaterialTextureType::Albedo, RendererGlobals::GetSingleColorTexture(glm::vec4(1, 1, 1, 1)));
			mat->SetTexture(MaterialTextureType::Mettalic, RendererGlobals::GetSingleColorTexture(glm::vec4(.5, .5, .5, 1)));
			mat->SetTexture(MaterialTextureType::Roughness, RendererGlobals::GetSingleColorTexture(glm::vec4(.5, .5, .5, 1)));
			mat->SetTexture(MaterialTextureType::Normal, RendererGlobals::GetSingleColorTexture(glm::vec4(0.5f, 0.5f, 1.f, 1.f)));

			mat->SetAlbedoColor(glm::vec3(1));
			mat->SetSpecular(glm::vec3(1.0f));

			Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices);
			mesh->SetDefaultMaterial(mat);
			s_LibraryData->Cube->m_Meshes.push_back(mesh);
		}

		//Sphere
		{
			s_LibraryData->Sphere = CreateRef<Model>();

			const int latitudeSegments = 32;
			const int longitudeSegments = 64;

			std::vector<MeshVertex> vertices;
			std::vector<uint16_t> indices;

			// Generate vertices
			for (int lat = 0; lat <= latitudeSegments; ++lat)
			{
				float theta = lat * glm::pi<float>() / latitudeSegments; // From 0 to PI
				float sinTheta = std::sin(theta);
				float cosTheta = std::cos(theta);

				for (int lon = 0; lon <= longitudeSegments; ++lon)
				{
					float phi = lon * 2.0f * glm::pi<float>() / longitudeSegments; // From 0 to 2*PI
					float sinPhi = std::sin(phi);
					float cosPhi = std::cos(phi);

					glm::vec3 normal = glm::vec3(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta);
					glm::vec2 texCoords = glm::vec2(float(lon) / longitudeSegments, float(lat) / latitudeSegments);

					// Tangent and bitangent generation (assume consistent winding)
					glm::vec3 tangent = glm::vec3(-sinPhi, 0.0f, cosPhi);
					glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

					vertices.push_back({
						normal,
						texCoords,
						glm::normalize(normal),
						glm::normalize(tangent),
						bitangent
						});
				}
			}

			for (int lat = 0; lat < latitudeSegments; ++lat)
			{
				for (int lon = 0; lon < longitudeSegments; ++lon)
				{
					int current = lat * (longitudeSegments + 1) + lon;
					int next = current + longitudeSegments + 1;

					indices.push_back(current);
					indices.push_back(next);
					indices.push_back(next + 1);

					indices.push_back(current);
					indices.push_back(next + 1);
					indices.push_back(current + 1);
				}
			}

			Ref<Material> mat = CreateRef<Material>(GetDeferredPassRenderPass());
			mat->SetTexture(MaterialTextureType::Albedo, RendererGlobals::GetSingleColorTexture(glm::vec4(1, 1, 1, 1)));
			//mat->SetTexture(MaterialTextureType::Albedo, TextureLibrary::Load("assets/textures/container2.png",ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN));
			mat->SetTexture(MaterialTextureType::Mettalic, RendererGlobals::GetSingleColorTexture(glm::vec4(.5, .5, .5, 1)));
			mat->SetTexture(MaterialTextureType::Roughness, RendererGlobals::GetSingleColorTexture(glm::vec4(.5, .5, .5, 1)));
			mat->SetTexture(MaterialTextureType::Normal, RendererGlobals::GetSingleColorTexture(glm::vec4(0.5f, 0.5f, 1.f, 1.f)));

			mat->SetAlbedoColor(glm::vec3(1));
			mat->SetSpecular(glm::vec3(1.0f));

			Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices);
			mesh->SetDefaultMaterial(mat);
			s_LibraryData->Sphere->m_Meshes.push_back(mesh);
		}
	}

	void ModelLibrary::Shutdown()
	{
		delete s_LibraryData;
	}
	Ref<Model> ModelLibrary::LoadModel(const Path& path)
	{
		//if (IsCached(path))
		//	return LoadFromCache(path);
		return LoadFromFile(path);
	}
	Ref<Model> ModelLibrary::GetModel(const Path& path)
	{
		return Ref<Model>();
	}
	Ref<Model> ModelLibrary::GetBaseCube()
	{
		return s_LibraryData->Cube;
	}
	Ref<Model> ModelLibrary::GetBaseSphere()
	{
		return s_LibraryData->Sphere;
	}
	Ref<Model> ModelLibrary::GetBaseCylinder()
	{
		return s_LibraryData->Cylinder;
	}
	bool ModelLibrary::IsLoaded(const Path& path)
	{
		return false;
	}
	bool ModelLibrary::IsCached(const Path& path)
	{
		return FileSystem::exists(GetCachePath(path));
	}
	Path ModelLibrary::GetCachePath(const Path& path)
	{
		return path.parent_path() / (path.stem().string() + ".kdsmesh");
	}
	Ref<Model> ModelLibrary::LoadFromCache(const Path& path)
	{
		std::ifstream cache(GetCachePath(path), std::ios::binary | std::ios::beg | std::ios::out);
		KD_CORE_ASSERT(cache.is_open());
		{
			char l[8];
			cache.read(l,7);
			l[7] = 0;
			KD_CORE_ASSERT(std::strcmp(l, "KDSMESH") == 0);
		}

		Ref<Model> model = CreateRef<Model>();
		model->m_ModelPath = path;
		model->m_ModelDir = path.parent_path();
		while(true){
			uint64_t numMeshes = 0;
			cache.read((char*)&numMeshes,sizeof(uint64_t));
			if(cache.eof())
				return model;
			for(uint64_t i = 0; i < numMeshes; ++i){
				
				uint64_t numVertices = 0;
				cache.read((char*)&numVertices, sizeof(uint64_t));
				std::vector<MeshVertex> vertices;
				vertices.resize(numVertices);
				cache.read((char*)vertices.data(), numVertices * sizeof(MeshVertex));

				uint64_t numIndices = 0;
				cache.read((char*)&numIndices, sizeof(uint64_t));
				std::vector<uint16_t> indices;
				indices.resize(numIndices);
				cache.read((char*)indices.data(), numIndices * sizeof(uint16_t));

				Ref<Material> mat = CreateRef<Material>(GetDeferredPassRenderPass());
				
				uint64_t textureCount = 0;
				cache.read((char*)&textureCount, sizeof(uint64_t));

				for(uint64_t t = 0; t < textureCount; ++t){
					uint32_t typeInt = 0;
					cache.read((char*)&typeInt, sizeof(uint32_t));
					MaterialTextureType type = (MaterialTextureType)typeInt;
					uint64_t length = 0;
					cache.read((char*)&length, sizeof(uint64_t));
					std::string path;
					path.resize(length);
					cache.read(path.data(), length);
					mat->SetTexture(type, TextureLibrary::Load(model->m_ModelDir / path, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN));
				}

				Ref<Mesh> mesh = CreateRef<Mesh>(vertices,indices);
				mesh->SetDefaultMaterial(mat);
				
				model->m_Meshes.push_back(mesh);
			}
		}
	}
	Ref<Model> ModelLibrary::LoadFromFile(const Path& path)
	{
		const aiScene* scene = nullptr;
		Assimp::Importer importer;
		
		{
			SCOPED_TIMER("Assimp ReadFile");
			scene =
				importer.ReadFile(path.string(),
					aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs);
		}
		


		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			KD_CORE_ASSERT(importer.GetErrorString());
		}

		Ref<Model> model = CreateRef<Model>();
		model->m_ModelDir = path.parent_path();
		model->m_ModelPath = path;

		std::ofstream cache(GetCachePath(path), std::ios::binary | std::ios::beg | std::ios::out);
		KD_CORE_ASSERT(cache.is_open());
		cache.write("KDSMESH",7);
		ProcessNode(model, scene, scene->mRootNode, cache);
		return model;
	}
	void ModelLibrary::ProcessNode(Ref<Model> model, const aiScene* scene, const aiNode* node, std::ostream& cache)
	{
		if(node->mNumMeshes){
			uint64_t count = node->mNumMeshes;
			cache.write((const char*)&count,sizeof(uint64_t));
		}
		for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(model, scene, mesh, cache);
		}

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			const aiNode* child = node->mChildren[i];
			ProcessNode(model, scene, child, cache);
		}
	}
	Ref<Mesh> ModelLibrary::ProcessMesh(Ref<Model> model, const aiScene* scene, const aiMesh* mesh, std::ostream& cache)
	{
		SCOPED_TIMER("ProcessMesh");
		std::vector<MeshVertex> vertices;
		vertices.resize(mesh->mNumVertices);
		std::vector<uint16_t> indices;
		{
			SCOPED_TIMER("ProcessVertices");
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
		}

		{
			SCOPED_TIMER("ProcessIndices");
			for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
				aiFace face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; ++j) {
					indices.push_back(face.mIndices[j]);
				}
			}
		}

		{
			uint64_t vertexCount = vertices.size();
			cache.write((const char*)&vertexCount, sizeof(vertexCount));
			cache.write((const char*)vertices.data(), vertexCount * sizeof(vertices[0]));
		}
		{
			uint64_t indexCount = indices.size();
			cache.write((const char*)&indexCount, sizeof(indexCount));
			cache.write((const char*)indices.data(), indexCount * sizeof(indices[0]));
		}
		Ref<Material> mat = {};
		{
			mat = ProcessMaterial(model, scene, mesh, scene->mMaterials[mesh->mMaterialIndex], cache);
		}

		Ref<Mesh> m = CreateRef<Mesh>(vertices, indices);

		m->SetDefaultMaterial(mat);

		return m;

	}
	Ref<Material> ModelLibrary::ProcessMaterial(Ref<Model> model, const aiScene* scene, const aiMesh* mesh, const aiMaterial* material, std::ostream& cache)
	{
		SCOPED_TIMER("ProcessMaterial");
		Ref<Texture2D> albedo = {};
		std::vector<std::pair<uint32_t, std::string>> texturePaths;
		if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			albedo = TextureLibrary::Load(model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Albedo;
				texturePaths.push_back({type, pathStr});
			}
		}

		Ref<Texture2D> metallic = {};
		if (material->GetTextureCount(aiTextureType_UNKNOWN)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			metallic = TextureLibrary::Load(model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Mettalic;
				texturePaths.push_back({type, pathStr});
			}
		}

		Ref<Texture2D> roughness = {};
		if (material->GetTextureCount(aiTextureType_UNKNOWN)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			roughness = TextureLibrary::Load(model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Roughness;
				texturePaths.push_back({type, pathStr});
			}
		}

		Ref<Texture2D> normals = {};
		if (material->GetTextureCount(aiTextureType_NORMALS)) {
			aiString texturePath;
			KD_CORE_ASSERT(material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == aiReturn_SUCCESS);
			KD_CORE_ASSERT(texturePath.length);

			std::string pathStr = texturePath.C_Str();

			normals = TextureLibrary::Load(model->m_ModelDir / pathStr, ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);

			{
				uint32_t type = (uint32_t)MaterialTextureType::Normal;
				texturePaths.push_back({type, pathStr});
			}
		}

		{
			uint64_t length = texturePaths.size();
			cache.write((const char*)&length, sizeof(uint64_t));
		}

		for(auto& [type, path] : texturePaths){
			uint64_t length = path.length();

			cache.write((const char*)&type,sizeof(uint32_t));
			cache.write((const char*)&length, sizeof(uint64_t));
			cache.write(path.c_str(), length);
		}


		Ref<Material> mat = CreateRef<Material>(GetDeferredPassRenderPass());
		mat->SetTexture(MaterialTextureType::Albedo, albedo);
		mat->SetTexture(MaterialTextureType::Mettalic, metallic);
		mat->SetTexture(MaterialTextureType::Roughness, roughness);
		mat->SetTexture(MaterialTextureType::Normal, normals);
		return mat;
	}
}
