#include "KDpch.h"

#include "AnimationSerializer.h"
#include <Kaidel/Core/YAMLHelpers.h>

#include "Kaidel/Core/EnumStringConvertor.h"

namespace Kaidel
{
	namespace Utils
	{
		DECL_CONVERTOR(ValueTypeToString, StringToValueType,
			AnimationValueType::Position, "Position",
			AnimationValueType::Rotation, "Rotation",
			AnimationValueType::Scale, "Scale");
	}

	template<typename T>
	static void SerializeTrack(YAML::Emitter& out, AnimationTrack& track, T ptrToVal)
	{
		out << YAML::BeginMap;
		
		out << YAML::Key << "Type" << YAML::Value << Utils::ValueTypeToString(track.ValueType);

		out << YAML::Key << "Frames" << YAML::Value << YAML::BeginSeq;

		for (auto& frame : track.Frames)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Time" << YAML::Value << frame.Time;

			out << YAML::Key << "Target" << YAML::Value;

			out << (frame.*ptrToVal).Target;

			out << YAML::EndMap;
		}

		out << YAML::EndSeq;

		out << YAML::EndMap;
	}
	
	

	static void SerializeTree(YAML::Emitter& out, AnimationTree::AnimationTreeNode& curr, const std::string& totalPath)
	{
		out << YAML::Key << totalPath << YAML::Value << YAML::BeginSeq;

		for (auto& track : curr.Tracks)
		{
			if (track)
			{
				switch (track->ValueType)
				{
				case AnimationValueType::Position: SerializeTrack(out, *track, &AnimationFrame::Position); break;
				case AnimationValueType::Rotation: SerializeTrack(out, *track, &AnimationFrame::Rotation); break;
				case AnimationValueType::Scale: SerializeTrack(out, *track, &AnimationFrame::Scale); break;
				}
			}
		}

		out << YAML::EndSeq;

		for (auto& [name, child] : curr.Children)
		{
			SerializeTree(out, child, totalPath + '/' + name);
		}
	}

	void AnimationSerailizer::Serialize(const Path& path, Ref<AnimationTree> anim)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;

		SerializeTree(out, anim->RootNode, ".");

		out << YAML::EndMap;

		std::ofstream file(path);

		file << out.c_str();
	}

	template<typename _ValueType, typename T>
	static std::vector<AnimationFrame> DeserializeFrames(YAML::Node& frames, T ptrToVal)
	{
		std::vector<AnimationFrame> res;
		res.reserve(frames.size());

		for (auto frame : frames)
		{
			AnimationFrame f;
			f.Time = frame["Time"].as<uint32_t>();
			(f.*ptrToVal).Target = frame["Target"].as<_ValueType>();

			res.emplace_back(f);
		}

		std::sort(res.begin(), res.end(), [](auto& a, auto& b) { return a.Time < b.Time; });

		return res;
	}

	static Scope<AnimationTrack> DeserializeTrack(YAML::Node& track)
	{
		AnimationValueType type = Utils::StringToValueType(track["Type"].as<std::string>());

		std::vector<AnimationFrame> frames;

		switch (type)
		{
		case Kaidel::AnimationValueType::Position: frames = DeserializeFrames<glm::vec3>(track["Frames"], &AnimationFrame::Position); break;
		case Kaidel::AnimationValueType::Rotation: frames = DeserializeFrames<glm::quat>(track["Frames"], &AnimationFrame::Rotation); break;
		case Kaidel::AnimationValueType::Scale: frames = DeserializeFrames<glm::vec3>(track["Frames"], &AnimationFrame::Scale); break;
		default:
			KD_CORE_ASSERT(false);
			break;
		}

		auto t = CreateScope<AnimationTrack>();

		t->Frames = std::move(frames);
		t->ValueType = type;

		return t;
	}

	void AnimationSerailizer::Deserialize(Ref<AnimationTree> anim, const Path& path)
	{
		YAML::Node node = YAML::Load(std::ifstream(path));

		for (auto treeNode : node)
		{
			Path path = treeNode.first.as<std::string>();

			auto current = path.begin();

			current++;

			auto tree = &anim->RootNode;

			while (current != path.end())
			{
				tree = &tree->Children[(*current).string()];
			}

			for (auto track : treeNode.second)
			{
				auto t = DeserializeTrack(track);

				tree->Tracks[(uint32_t)t->ValueType] = std::move(t);
			}
		}
	}

}
