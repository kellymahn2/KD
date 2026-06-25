#include "KDpch.h"
#include "ModelLibrary.h"
#include "ModelLoader.h"
#include "Kaidel/Renderer/Renderer3D.h"
#include "Kaidel/Asset/AssetManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Kaidel {

	static struct ModelLibraryData {
		std::unordered_map<Path, Ref<Model>> LoadedModels;
		Ref<Mesh> Cube;
		Ref<Mesh> Sphere;
		Ref<Mesh> Cylinder;
	}* s_LibraryData;

	void ModelLibrary::Init()
	{
		s_LibraryData = new ModelLibraryData;

		//Cube
		{
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
			const std::vector<uint16_t> indices = {
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

			Ref<StandardMaterialInstance> mat = RendererGlobals::GetDefaultMaterial();
			
			Submesh submesh;
			submesh.DefaultMaterial = mat;
			submesh.IndexCount = indices.size();
			submesh.VertexCount = vertices.size();
			submesh.BoundingBox.Min = glm::vec3(-1.0f, -1.0f, -1.0f);
			submesh.BoundingBox.Max = glm::vec3(1.0f, 1.0f, 1.0f);
			submesh.IndexOffset = submesh.VertexOffset = 0;

			MeshInitializer initializer;
			initializer.Submeshes.push_back(submesh);
			initializer.VertexCount = vertices.size();
			initializer.IndexCount = indices.size();
			initializer.IndexFormat = IndexType::Uint16;
			initializer.VertexSize = sizeof(MeshVertex);
			initializer.Indices = indices.data();
			initializer.Vertices = vertices.data();

			Ref<Mesh> mesh = CreateRef<Mesh>(initializer);
			s_LibraryData->Cube = mesh;

			AssetManager::RegisterBuiltIn(s_LibraryData->Cube, "Cube");
		}

		//Sphere
		{
			const float pi = glm::pi<float>();

			const uint32_t lats = 32;
			const uint32_t lons = 32;
			
			const float radius = 1.0;

			std::vector<MeshVertex> vertices;
			std::vector<uint16_t> indices;

			for (uint32_t latIndex = 0; latIndex <= lats; ++latIndex)
			{
				float v = static_cast<float>(latIndex) / lats;
				float lat = v * pi;

				float cosLat = std::cos(lat);
				float sinLat = std::sin(lat);

				for (uint32_t lonIndex = 0; lonIndex <= lons; ++lonIndex)
				{
					float u = static_cast<float>(lonIndex) / lons;
					float lon = u * 2.0f * pi;

					float cosLon = std::cos(lon);
					float sinLon = std::sin(lon);

					// ---- Vertex ----
					MeshVertex vertex{};

					const float x = radius * sinLat * cosLon;
					const float y = radius * sinLat * sinLon;
					const float z = radius * cosLat;

					vertex.Position = glm::vec3(
						x, y, z
					);

					vertex.ModelNormal = glm::normalize(vertex.Position);

					vertex.ModelTangent = glm::vec3(-radius *  sinLon,
						radius * cosLon,
						0.0f);
					vertex.ModelTangent = glm::normalize(vertex.ModelTangent);

					vertex.ModelBitangent = glm::normalize(glm::cross(vertex.ModelNormal, vertex.ModelTangent));

					vertex.TexCoords = glm::vec2(u, v);

					vertices.emplace_back(vertex);
				}
			}

			uint16_t sqrCount = (lats + 1) * (lons + 1);


			for (uint16_t i = 0; i < sqrCount; ++i)
			{
				uint16_t topLeft = i;
				uint16_t topRight = i + 1;

				uint16_t bottomLeft = i + (lons + 1);
				uint16_t bottomRight = i + (lons + 1) + 1;

				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
				indices.push_back(topRight);

				indices.push_back(topRight);
				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
			}

			Ref<StandardMaterialInstance> mat = RendererGlobals::GetDefaultMaterial();

			Submesh submesh;
			submesh.DefaultMaterial = mat;
			submesh.IndexCount = indices.size();
			submesh.VertexCount = vertices.size();
			submesh.BoundingBox.Min = glm::vec3(-1.0f, -1.0f, -1.0f);
			submesh.BoundingBox.Max = glm::vec3(1.0f, 1.0f, 1.0f);
			submesh.IndexOffset = submesh.VertexOffset = 0;

			MeshInitializer initializer;
			initializer.Submeshes.push_back(submesh);
			initializer.VertexCount = vertices.size();
			initializer.IndexCount = indices.size();
			initializer.IndexFormat = IndexType::Uint16;
			initializer.VertexSize = sizeof(MeshVertex);
			initializer.Indices = indices.data();
			initializer.Vertices = vertices.data();


			Ref<Mesh> mesh = CreateRef<Mesh>(initializer);
			s_LibraryData->Sphere = mesh;

			AssetManager::RegisterBuiltIn(s_LibraryData->Sphere, "Sphere");
			//s_LibraryData->Sphere = ModelLibrary::LoadModel("assets/models/uvsphere.gltf")->GetMeshTree()->NodeMesh;
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
		//ModelLoader loader;
		//return loader.Load(path);

		return {};

	}
	Ref<Model> ModelLibrary::GetModel(const Path& path)
	{
		return Ref<Model>();
	}
	Ref<Mesh> ModelLibrary::GetBaseCube()
	{
		return s_LibraryData->Cube;
	}
	Ref<Mesh> ModelLibrary::GetBaseSphere()
	{
		return s_LibraryData->Sphere;
	}
	Ref<Mesh> ModelLibrary::GetBaseCylinder()
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
}
