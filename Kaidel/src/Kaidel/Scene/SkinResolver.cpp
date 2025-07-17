#include "KDpch.h"
#include "SkinResolver.h"
#include <glm/gtc/type_ptr.hpp>
namespace Kaidel {

	namespace Utils {
		static bool IsEqual(const glm::mat4& m1, const glm::mat4& m2)
		{
			return
				glm::all(glm::equal(m1[0], m2[0])) &&
				glm::all(glm::equal(m1[1], m2[1])) &&
				glm::all(glm::equal(m1[2], m2[2]))&&
				glm::all(glm::equal(m1[3], m2[3]));
		}

		static bool IsSameTree(const SkinTree* tree1, const SkinTree* tree2)
		{
			if (tree1->Name != tree2->Name)
				return false;

			if (!IsEqual(tree1->BindMatrix, tree2->BindMatrix))
				return false;

			if (tree1->Children.size() != tree2->Children.size())
				return false;

			for (uint32_t i = 0; i < tree1->Children.size(); ++i)
			{
				if (!IsSameTree(&tree1->Children[i], &tree2->Children[i]))
					return false;
			}

			return true;
		}

	}


	SkinResolver::SkinResolver(const aiScene* scene)
		: m_Scene(scene)
	{
		BuildNodeMaps(scene->mRootNode, 0);
	}

	std::unordered_map<const aiMesh*, Ref<SkinTree>> SkinResolver::ResolveSkinTrees()
	{
		std::unordered_map<const aiMesh*, Ref<SkinTree>> result;

		for (uint32_t i = 0; i < m_Scene->mNumMeshes; ++i) {
			const aiMesh* mesh = m_Scene->mMeshes[i];

			if (!mesh->HasBones())
				continue;

			Ref<SkinTree> skinTree = BuildSkinTree(mesh);

			auto tree = std::find_if(m_Skins.begin(), m_Skins.end(), [skinTree](const Ref<SkinTree>& tree)
				{
					return Utils::IsSameTree(skinTree.Get(), tree.Get());
				});

			if (tree == m_Skins.end()) {
				m_Skins.push_back(skinTree);
				result[mesh] = skinTree;
			}
			else {
				result[mesh] = *tree;
			}

		}

		return result;
	}

	void SkinResolver::BuildNodeMaps(const aiNode* node, uint32_t depth)
	{
		m_NodeNameMap[node->mName.C_Str()] = node;
		m_NodeDepthMap[node] = depth;

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			BuildNodeMaps(node->mChildren[i], depth + 1);
		}
	}

	Ref<SkinTree> SkinResolver::BuildSkinTree(const aiMesh* mesh)
	{
		const aiNode* rootBone = FindRootBone(mesh);

		std::unordered_map<const aiNode*, const aiBone*> nodeToBone;

		for (uint32_t i = 0; i < mesh->mNumBones; ++i)
		{
			const aiBone* bone = mesh->mBones[i];
			const aiNode* node = m_NodeNameMap[bone->mName.C_Str()];

			nodeToBone[node] = bone;
		}

		Ref<SkinTree> tree = CreateRef<SkinTree>();
		uint32_t index = 0;
		BuildSkinTree(mesh, rootBone, tree.Get(), nodeToBone, index);

		return tree;
	}

	void SkinResolver::BuildSkinTree(
		const aiMesh* mesh, const aiNode* currNode, SkinTree* outTree, 
		const std::unordered_map<const aiNode*, const aiBone*>& nodeToBone, uint32_t& index)
	{
		const aiBone* bone = nodeToBone.at(currNode);

		outTree->BindMatrix = glm::transpose(*(const glm::mat4*)&bone->mOffsetMatrix);
		outTree->Name = bone->mName.C_Str();
		outTree->ID = index;
		++index;

		if (currNode->mNumChildren)
		{
			outTree->Children.resize(currNode->mNumChildren);

			for (uint32_t i = 0; i < currNode->mNumChildren; ++i)
			{
				BuildSkinTree(mesh, currNode->mChildren[i], &outTree->Children[i], nodeToBone, index);
			}
		}
	}

	const aiNode* SkinResolver::FindRootBone(const aiMesh* mesh)
	{
		const aiNode* rootBone = nullptr;
		uint32_t minDepth = UINT_MAX;

		for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
			const aiBone* bone = mesh->mBones[i];

			const aiNode* node = m_NodeNameMap[bone->mName.C_Str()];
			uint32_t depth = m_NodeDepthMap[node];

			if (depth < minDepth)
			{
				minDepth = depth;
				rootBone = node;
			}
		}

		KD_CORE_ASSERT(rootBone);

		return rootBone;
	}
}
