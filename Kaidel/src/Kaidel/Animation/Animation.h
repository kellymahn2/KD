#pragma once
#include "Kaidel/Core/Base.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Kaidel {
	enum class AnimationValueType : uint32_t {
		Position,
		Rotation,
		Scale,

		MaxValue,
	};

	static const constexpr uint32_t AnimationValueTypeCount = (uint32_t)AnimationValueType::MaxValue;

	enum class AnimationInterpolationType {
		Linear
	};
	
	struct AnimationPositionData {
		glm::vec3 Target;
	};

	struct AnimationRotationData {
		glm::quat Target;
	};

	struct AnimationScaleData {
		glm::vec3 Target;
	};

	struct AnimationFrame {
		union {
			AnimationPositionData Position;
			AnimationRotationData Rotation;
			AnimationScaleData Scale;
		};
		float Time;

		AnimationFrame(const AnimationPositionData& position, float time)
			: Position(position), Time(time)
		{ 
		}
		AnimationFrame(const AnimationRotationData& rotation, float time)
			: Rotation(rotation), Time(time)
		{
		}
		AnimationFrame(const AnimationScaleData& scale, float time)
			: Scale(scale), Time(time)
		{ 
		}
	};

	struct AnimationTrack {
		AnimationValueType ValueType = AnimationValueType::Position;
		AnimationInterpolationType InterpolationType = AnimationInterpolationType::Linear;
		std::vector<AnimationFrame> Frames;
	};

	struct AnimationTree : IRCCounter<false> {
		struct AnimationTreeNode {
			Scope<AnimationTrack> Tracks[AnimationValueTypeCount];
			std::unordered_map<std::string, AnimationTreeNode> Children;
		};

		AnimationTreeNode RootNode;
		float Duration;
	};
}
