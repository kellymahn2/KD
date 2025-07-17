#pragma once
#include "Kaidel/Core/Base.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <assimp/scene.h>

namespace Kaidel {

	class SkinResolver {
	public:
		SkinResolver() = default;
		SkinResolver(const aiScene* scene);
		std::unordered_map<const aiMesh*, Ref<SkinTree>> ResolveSkinTrees();
		
		const auto& GetUniqueSkins()const { return m_Skins; }

	private:
		const aiScene* m_Scene;
		std::unordered_map<std::string, const aiNode*> m_NodeNameMap;
		std::unordered_map<const aiNode*, size_t> m_NodeDepthMap;
		std::vector<Ref<SkinTree>> m_Skins;
		
	private:
		void BuildNodeMaps(const aiNode* node, uint32_t depth);

		Ref<SkinTree> BuildSkinTree(const aiMesh* mesh);
		void BuildSkinTree(
			const aiMesh* mesh, const aiNode* currNode, SkinTree* outTree, 
			const std::unordered_map<const aiNode*, const aiBone*>& nodeToBone, uint32_t& index);

		const aiNode* FindRootBone(const aiMesh* mesh);
	};
}
