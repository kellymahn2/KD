#include "KDpch.h"
#include "AnimationResolver.h"

namespace Kaidel {


	AnimationResolver::AnimationResolver(const aiScene* scene)
		: m_Scene(scene)
	{
		BuildNodeMaps(scene->mRootNode, 0, std::list<std::string>());
	}

	std::unordered_map<const aiNode*, Ref<AnimationTree>> AnimationResolver::ResolveAnimations()
	{
		std::unordered_map<const aiNode*, Ref<AnimationTree>> res;

		for (uint32_t i = 0; i < m_Scene->mNumAnimations; ++i) 
		{
			const aiAnimation* animation = m_Scene->mAnimations[i];

			const aiNode* animRootNode = FindRootNode(animation);

			Ref<AnimationTree> tree = ProcessAnimation(animation, animRootNode);
			res[animRootNode] = tree;
		}

		return res;
	}


	const aiNode* AnimationResolver::FindRootNode(const aiAnimation* animation)
	{
		const aiNode* root = nullptr;
		uint32_t rootDepth = UINT32_MAX;

		for (uint32_t i = 0; i < animation->mNumChannels; ++i)
		{
			const aiNodeAnim* nodeAnim = animation->mChannels[i];
			
			const aiNode* node = m_NodeNameMap[nodeAnim->mNodeName.C_Str()];
			uint32_t depth = m_NodeDepthMap[node];

			if (depth < rootDepth)
			{
				root = node;
				rootDepth = depth;
			}
		}

		return root;
	}

	void AnimationResolver::BuildNodeMaps(const aiNode* node, uint32_t depth, std::list<std::string> currPath)
	{
		m_NodeNameMap[node->mName.C_Str()] = node;
		m_NodeDepthMap[node] = depth;
		currPath.push_back(node->mName.C_Str());
		m_NodePathMap[node] = std::move(currPath);

		for (uint32_t i = 0; i < node->mNumChildren; ++i) {
			BuildNodeMaps(node->mChildren[i], depth + 1, m_NodePathMap[node]);
		}
	}

	AnimationTree::AnimationTreeNode& AnimationResolver::FindOrCreateMidTrees(
		const aiNode* animRootNode, const aiNodeAnim* nodeAnim, AnimationTree::AnimationTreeNode& rootNode)
	{
		const aiNode* currNode = m_NodeNameMap[nodeAnim->mNodeName.C_Str()];

		const std::list<std::string>& nodePath = m_NodePathMap[currNode];

		AnimationTree::AnimationTreeNode* currAnimNode = &rootNode;

		auto it = nodePath.begin();

		while (*it != animRootNode->mName.C_Str())
			++it;

		++it;

		for (; it != nodePath.end(); ++it)
		{
			const std::string& currNodeName = *it;

			currAnimNode = &currAnimNode->Children[currNodeName];
		}

		return *currAnimNode;
	}

	Ref<AnimationTree> AnimationResolver::ProcessAnimation(const aiAnimation* animation, const aiNode* animRootNode)
	{
		Ref<AnimationTree> tree = CreateRef<AnimationTree>();

		float timeScale = 1.0f / (float)(animation->mTicksPerSecond != 0.0 ? animation->mTicksPerSecond : 30.0);
		
		tree->Duration = animation->mDuration * timeScale;

		for (uint32_t i = 0; i < animation->mNumChannels; ++i)
		{
			const aiNodeAnim* nodeAnim = animation->mChannels[i];

			AnimationTree::AnimationTreeNode& currNode = FindOrCreateMidTrees(animRootNode, nodeAnim, tree->RootNode);

			{
				auto& posTrack = currNode.Tracks[(uint32_t)AnimationValueType::Position] = CreateScope<AnimationTrack>();
				posTrack->InterpolationType = AnimationInterpolationType::Linear;
				posTrack->ValueType = AnimationValueType::Position;

				for (uint32_t i = 0; i < nodeAnim->mNumPositionKeys; ++i) {
					const aiVectorKey& key = nodeAnim->mPositionKeys[i];

					float time = key.mTime * timeScale;

					AnimationFrame frame(AnimationPositionData{ glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z) }, time);
					posTrack->Frames.emplace_back(frame);
				}
			}
			
			{
				auto& rotTrack = currNode.Tracks[(uint32_t)AnimationValueType::Rotation] = CreateScope<AnimationTrack>();
				rotTrack->InterpolationType = AnimationInterpolationType::Linear;
				rotTrack->ValueType = AnimationValueType::Rotation;

				for (uint32_t i = 0; i < nodeAnim->mNumRotationKeys; ++i) {
					auto key = nodeAnim->mRotationKeys + i;

					float time = key->mTime * timeScale;

					glm::quat q = glm::normalize(glm::quat(key->mValue.w, key->mValue.x, key->mValue.y, key->mValue.z));
					AnimationFrame frame(AnimationRotationData{ q }, time);
					rotTrack->Frames.emplace_back(frame);
				}
			}

			{
				auto& sclTrack = currNode.Tracks[(uint32_t)AnimationValueType::Scale] = CreateScope<AnimationTrack>();
				sclTrack->InterpolationType = AnimationInterpolationType::Linear;
				sclTrack->ValueType = AnimationValueType::Scale;

				for (uint32_t i = 0; i < nodeAnim->mNumScalingKeys; ++i) {
					const aiVectorKey& key = nodeAnim->mScalingKeys[i];
					
					float time = key.mTime * timeScale;

					AnimationFrame frame(AnimationPositionData{ glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z) }, time);
					sclTrack->Frames.emplace_back(frame);
				}
			}
		}

		return tree;
	}
}
