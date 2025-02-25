#pragma once

#include "Kaidel/Core/Base.h"

#include <glm/glm.hpp>
namespace Kaidel {
	struct AnimationSubKey {
		glm::vec2 Point = {};
		AnimationSubKey() = default;
		AnimationSubKey(float time, float value) 
			: Point(time,value) { }
	};
	struct AnimationKey {
		AnimationSubKey BeforeKey, MainKey, AfterKey;
	};

	class AnimationProperty {
	public:
		AnimationProperty() = default;

		float Interpolate(float time, uint32_t i);

		void PushKey(const AnimationSubKey& key);
		//Splits the curve at point t.
		void SplitCurve(float time);

		std::vector<AnimationKey>& GetKeys() { return m_Keys; }
	private:
		std::vector<AnimationKey> m_Keys;
	};
}
