#include "KDpch.h"
#include "ModelLibrary.h"
#include "ModelLoader.h"
#include "Kaidel/Renderer/Renderer3D.h"

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
			const std::vector<unsigned int> indices = {
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

			//Ref<Material> mat = CreateRef<Material>(GetDeferredPassRenderPass());
			//mat->SetRoughness(1.0f);
			//mat->SetMetalness(0.0f);
			
			/*Submesh submesh;
			submesh.DefaultMaterial = mat;
			submesh.IndexCount = indices.size();
			submesh.VertexCount = vertices.size();
			submesh.BoundingBox.Min = glm::vec3(-1.0f, -1.0f, -1.0f);
			submesh.BoundingBox.Max = glm::vec3(1.0f, 1.0f, 1.0f);*/

			/*Ref<Mesh> mesh = CreateRef<Mesh>(vertices, indices, std::vector<Submesh>{submesh});
			s_LibraryData->Cube = mesh;*/
		}

		//Sphere
		{
			float pi = glm::pi<float>();
			float tau = 2.0f * pi;

			int lats = 32;
			int lons = 32;
			
			const double lat_step = tau / lats;
			const double lon_step = tau / lons;
			float radius = 1.0;

			std::vector<MeshVertex> vertices;
			std::vector<uint32_t> indices;

			glm::mat3 rotate = glm::mat3(glm::rotate(glm::mat4(1.0f), pi * 0.5f, glm::vec3(0, 1, 0)));

			for (int i = 1; i <= lats; i++) {
				double lat0 = lat_step * (i - 1) - tau/ 4;
				double z0 = glm::sin(lat0);
				double zr0 = glm::cos(lat0);

				double lat1 = lat_step * i - tau / 4;
				double z1 = glm::sin(lat1);
				double zr1 = glm::cos(lat1);

				for (int j = lons; j >= 1; j--) {
					double lng0 = lon_step * (j - 1);
					double x0 = glm::cos(lng0);
					double y0 = glm::sin(lng0);

					double lng1 = lon_step * j;
					double x1 = glm::cos(lng1);
					double y1 = glm::sin(lng1);

					glm::vec3 v[4] = {
						glm::vec3(x1 * zr0, z0, y1 * zr0),
						glm::vec3(x1 * zr1, z1, y1 * zr1),
						glm::vec3(x0 * zr1, z1, y0 * zr1),
						glm::vec3(x0 * zr0, z0, y0 * zr0)
					};

					uint32_t offset = (uint32_t)vertices.size();
#define ADD_POINT(m_idx)                                                                       \
	{																								\
		MeshVertex vertex = {};																		\
		vertex.Position = (v[m_idx] * radius);                                                               \
		vertex.ModelNormal = glm::normalize(v[m_idx]);                                                     \
		{                                                                                          \
			glm::vec2 uv(std::atan2f(v[m_idx].x, v[m_idx].z), std::atan2f(-v[m_idx].y, v[m_idx].z)); \
			uv /= pi;                                                                         \
			uv *= 4.0f;                                                                             \
			uv = uv * 0.5f + glm::vec2(0.5f, 0.5f);                                                     \
			vertex.TexCoords = (uv);                                                                     \
		}                                                                                          \
		{                                                                                          \
			vertex.ModelTangent = glm::normalize(rotate * (v[m_idx]));                                                        \
			vertex.ModelBitangent = glm::normalize(glm::cross(vertex.ModelTangent, vertex.ModelNormal));\
		}																						   \
		vertices.push_back(vertex);\
		indices.push_back(offset + m_idx);\
	}

					ADD_POINT(0);
					ADD_POINT(1);
					ADD_POINT(2);

					ADD_POINT(2);
					ADD_POINT(3);
					ADD_POINT(0);
				}
			}

			/*Ref<Material> mat = CreateRef<Material>(Renderer3D::GetDeferredPassRenderPass());

			Submesh submesh;
			submesh.DefaultMaterial = mat;
			submesh.IndexCount = indices.size();
			submesh.VertexCount = vertices.size();
			submesh.IndexOffset = 0;
			submesh.VertexOffset = 0;
			submesh.BoundingBox.Min = glm::vec3(-1.0f, -1.0f, -1.0f);
			submesh.BoundingBox.Max = glm::vec3(1.0f, 1.0f, 1.0f);

			MeshInitializer initializer;
			initializer.Indices = indices.data();
			initializer.IndexCount = indices.size();
			initializer.Vertices = vertices.data();
			initializer.VertexCount = vertices.size();
			initializer.IndexFormat = IndexType::Uint16;
			initializer.VertexSize = sizeof(MeshVertex);
			initializer.Submeshes = { submesh };


			Ref<Mesh> mesh = CreateRef<Mesh>(initializer);*/
			s_LibraryData->Sphere = ModelLibrary::LoadModel("assets/models/uvsphere.gltf")->GetMeshTree()->NodeMesh;
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
		ModelLoader loader;
		return loader.Load(path);
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
