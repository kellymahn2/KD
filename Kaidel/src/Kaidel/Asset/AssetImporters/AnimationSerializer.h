#pragma once
#include "Kaidel/Animation/Animation.h"


namespace Kaidel
{
	class AnimationSerailizer {
	public:
		void Serialize(const Path& path, Ref<AnimationTree> anim);
		void Deserialize(Ref<AnimationTree> anim, const Path& path);
	};
}
