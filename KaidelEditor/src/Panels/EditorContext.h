#pragma once

#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Animation/Animation.h"

namespace Kaidel {

	class EditorContext
	{
	public:
		static void Init();
		static void Shutdown();
		
		static Entity SelectedEntity();
		static void SelectedEntity(Entity selectedEntity);

		static AnimationFrame* SelectedAnimationFrame();
		static AnimationValueType SelectedAnimationFrameValueType();
		static void SelectedAnimationFrame(AnimationFrame* selectedAnimationFrame, AnimationValueType type);
	};
}
