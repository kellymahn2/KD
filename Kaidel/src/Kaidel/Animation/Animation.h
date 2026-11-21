#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Asset/Asset.h"
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
		uint32_t Time;

		AnimationFrame() = default;

		AnimationFrame(const AnimationPositionData& position, uint32_t time)
			: Position(position), Time(time)
		{ 
		}
		AnimationFrame(const AnimationRotationData& rotation, uint32_t time)
			: Rotation(rotation), Time(time)
		{
		}
		AnimationFrame(const AnimationScaleData& scale, uint32_t time)
			: Scale(scale), Time(time)
		{ 
		}
	};

	struct AnimationTrack {
		AnimationValueType ValueType = AnimationValueType::Position;
		AnimationInterpolationType InterpolationType = AnimationInterpolationType::Linear;
		std::vector<AnimationFrame> Frames;
	};

	struct AnimationTree : AssetBase {
		struct AnimationTreeNode {
			Scope<AnimationTrack> Tracks[AnimationValueTypeCount];
			std::unordered_map<std::string, AnimationTreeNode> Children;
		};

		AnimationTreeNode RootNode;
		float Duration;

		DECL_ASSET_TYPE(Animation);
	};
}
