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

	static std::vector<std::pair<float, glm::vec3>> 
		FilterVec3Keys(const aiVectorKey* vectorKeys, uint32_t keyCount,  float timeScale)
	{
		std::vector<std::pair<float, glm::vec3>> frames;

		for (uint32_t i = 0; i < keyCount; ++i)
		{
			const aiVectorKey& key = vectorKeys[i];

			glm::vec3 value = glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z);

			if (!frames.empty())
			{
				auto& last = frames.back();

				if (glm::all(glm::equal(last.second, value, 0.01f)))
				{
					continue;
				}
			}

			frames.emplace_back(std::make_pair(key.mTime * timeScale, value));
		}

		return frames;
	}

	static std::vector<std::pair<float, glm::quat>>
		FilterQuatKeys(const aiQuatKey* quatKeys, uint32_t keyCount, float timeScale)
	{
		std::vector<std::pair<float, glm::quat>> frames;

		for (uint32_t i = 0; i < keyCount; ++i)
		{
			const aiQuatKey& key = quatKeys[i];

			glm::quat value = glm::quat(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);

			if (!frames.empty())
			{
				auto& last = frames.back();

				if (glm::all(glm::equal(last.second, value, 0.01f)))
				{
					continue;
				}
			}

			frames.emplace_back(std::make_pair(key.mTime * timeScale, value));
		}

		return frames;
	}

	Ref<AnimationTree> AnimationResolver::ProcessAnimation(const aiAnimation* animation, const aiNode* animRootNode)
	{
		Ref<AnimationTree> tree = CreateRef<AnimationTree>();

		float timeScale = 1.0f / (float)(animation->mTicksPerSecond != 0.0 ? animation->mTicksPerSecond : 30.0);
		
		tree->Duration = animation->mDuration * timeScale;

		//TODO: remove duplicates

		for (uint32_t i = 0; i < animation->mNumChannels; ++i)
		{
			const aiNodeAnim* nodeAnim = animation->mChannels[i];
			AnimationTree::AnimationTreeNode& currNode = FindOrCreateMidTrees(animRootNode, nodeAnim, tree->RootNode);

			{
				std::vector<AnimationFrame> frames;
				
				auto& posTrack = currNode.Tracks[(uint32_t)AnimationValueType::Position] = CreateScope<AnimationTrack>();
				posTrack->InterpolationType = AnimationInterpolationType::Linear;
				posTrack->ValueType = AnimationValueType::Position;
				
				auto uniqueFrames = FilterVec3Keys(nodeAnim->mPositionKeys, nodeAnim->mNumPositionKeys, timeScale);
				if (uniqueFrames.size() >= 1)
				{
					for (auto& [time, value] : uniqueFrames)
					{
						AnimationFrame frame(AnimationPositionData{ value }, time);
						frames.emplace_back(frame);
					}

					posTrack->Frames = std::move(frames);
				}
				else
				{
					currNode.Tracks[(uint32_t)AnimationValueType::Position] = {};
				}
			}
			
			{
				std::vector<AnimationFrame> frames;

				auto& rotTrack = currNode.Tracks[(uint32_t)AnimationValueType::Rotation] = CreateScope<AnimationTrack>();
				rotTrack->InterpolationType = AnimationInterpolationType::Linear;
				rotTrack->ValueType = AnimationValueType::Rotation;

				auto uniqueFrames = FilterQuatKeys(nodeAnim->mRotationKeys, nodeAnim->mNumRotationKeys, timeScale);
				if (uniqueFrames.size() >= 1)
				{
					for (auto& [time, value] : uniqueFrames)
					{
						AnimationFrame frame(AnimationRotationData{ value }, time);
						frames.emplace_back(frame);
					}

					rotTrack->Frames = std::move(frames);
				}
				else
				{
					currNode.Tracks[(uint32_t)AnimationValueType::Rotation] = {};
				}
			}

			{
				std::vector<AnimationFrame> frames;

				auto& sclTrack = currNode.Tracks[(uint32_t)AnimationValueType::Scale] = CreateScope<AnimationTrack>();
				sclTrack->InterpolationType = AnimationInterpolationType::Linear;
				sclTrack->ValueType = AnimationValueType::Scale;

				auto uniqueFrames = FilterVec3Keys(nodeAnim->mScalingKeys, nodeAnim->mNumScalingKeys, timeScale);
				if (uniqueFrames.size() >= 1)
				{
					for (auto& [time, value] : uniqueFrames)
					{
						AnimationFrame frame(AnimationScaleData{ value }, time);
						frames.emplace_back(frame);
					}

					sclTrack->Frames = std::move(frames);
				}
				else
				{
					currNode.Tracks[(uint32_t)AnimationValueType::Scale] = {};
				}
			}
		}

		return tree;
	}
}
