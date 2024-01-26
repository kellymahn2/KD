#pragma once
#include "Asset.h"

#include "Kaidel/Mesh/Model.h"
#include "Kaidel/Renderer/Material.h"

namespace Kaidel {

	struct ManagedAssets {
		std::vector<Asset<Model>> Models;
		std::vector<Asset<Mesh>> Meshes;
		std::vector<Asset<Material>> Materials;
	};

	class AssetManager{
	public:


	private:
		static inline  ManagedAssets s_ManagedAssets;
	};

}
