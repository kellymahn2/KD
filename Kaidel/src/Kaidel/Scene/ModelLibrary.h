#pragma once
#include "Kaidel/Core/Base.h"
#include "Model.h"

/*
	TextureCache {
		Type : u32,
		Path : char*
	}
	MaterialCache {
		TextureCount : u64,
		TextureArray : TextureCache*
	}
	MeshCache {
		NumVertices : u64,
		VerticesArray : MeshVertex*,
		NumIndices : u64,
		IndicesArray : u16*,
		Material : MaterialCache
	}
	ModelCache {
		KDSMESH : string,
		NumMeshes : u64,
		MeshArray : MeshCache*
		...
	}
*/
namespace Kaidel {
	class ModelLibrary {
	public:
		static void Init();
		static void Shutdown();

		static Ref<Model> LoadModel(const Path& path);
		static Ref<Model> GetModel(const Path& path);
		
		static Ref<Mesh> GetBaseCube();
		static Ref<Mesh> GetBaseSphere();
		static Ref<Mesh> GetBaseCylinder();

		static bool IsLoaded(const Path& path);
	private:
		static bool IsCached(const Path& path);

		static Path GetCachePath(const Path& path);

		static Ref<Model> LoadFromCache(const Path& path);
		static Ref<Model> LoadFromFile(const Path& path);

		static void ProcessNode(Ref<Model> model, const aiScene* scene, const aiNode* node, MeshTree& outTree);
		static Ref<Mesh> ProcessMesh(Ref<Model> model, const aiScene* scene, const aiMesh* mesh);
		static Ref<Material> ProcessMaterial(Ref<Model> model, const aiScene* scene, const aiMesh* mesh, const aiMaterial* material);

		static std::vector<MeshVertex> ProcessVertices(const aiMesh* mesh);
		static std::vector<SkinnedMeshVertex> ProcessSkinnedVertices(const aiMesh* mesh);
		static std::vector<uint16_t> ProcessIndices(const aiMesh* mesh);
	};
}
