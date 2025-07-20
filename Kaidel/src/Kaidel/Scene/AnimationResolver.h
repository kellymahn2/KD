#pragma once
#include "Kaidel/Animation/Animation.h"

#include <assimp/scene.h>

namespace Kaidel {
	class AnimationResolver {
	public:
		AnimationResolver() = default;
		AnimationResolver(const aiScene* scene);

		std::unordered_map<const aiNode*, Ref<AnimationTree>> ResolveAnimations();

	private:
		const aiNode* FindRootNode(const aiAnimation* animation);
		void BuildNodeMaps(const aiNode* node, uint32_t depth, std::list<std::string> currPath);
		AnimationTree::AnimationTreeNode& FindOrCreateMidTrees(const aiNode* animRootNode, const aiNodeAnim* nodeAnim, AnimationTree::AnimationTreeNode& rootNode);
		Ref<AnimationTree> ProcessAnimation(const aiAnimation* animation, const aiNode* animRootNode);
	private:
		const aiScene* m_Scene;
		std::unordered_map<std::string, const aiNode*> m_NodeNameMap;
		std::unordered_map<const aiNode*, size_t> m_NodeDepthMap;
		std::unordered_map<const aiNode*, std::list<std::string>> m_NodePathMap;
	};
}
