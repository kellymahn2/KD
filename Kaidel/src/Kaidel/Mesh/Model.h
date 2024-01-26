#pragma once
#include "Mesh.h"
#include "Kaidel/Math/BoundingBox.h"
#include "Kaidel/Core/RecursiveTree.h"
#include "Kaidel/Assets/Asset.h"
#include <filesystem>
#include <unordered_map>
struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
enum aiTextureType;
namespace Kaidel {


	struct ModelData {
		std::string ModelName;
		std::vector<Mesh> Meshes;
	};

	class Model {
	public:
		static Ref<Model> Load(const std::filesystem::path& modelPath,bool flipUVs = false);
		void Draw(const glm::mat4& transform,Ref<Material>& mat,const Math::Frustum& frustum);
		const FileSystem::path& GetModelPath()const { return m_ModelPath; }

		const auto& GetModelData()const { return m_MeshHandles; }


		void Flush();
	private:

		std::unordered_map<uint32_t, Ref<Material>> LoadMaterials(const aiScene* scene, std::unordered_map<uint32_t, uint32_t>& embeddedTextures);

		std::vector<AssetHandle<Mesh>> m_MeshHandles;
		FileSystem::path m_ModelPath;

		uint64_t m_MeshCount;


		friend class Scene;
	};

}
