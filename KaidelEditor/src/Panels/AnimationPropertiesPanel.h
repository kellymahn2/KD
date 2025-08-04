#pragma once
#include "Kaidel/Animation/Animation.h"

namespace Kaidel {
	class AnimationPropertiesPanel {
	public:

		AnimationPropertiesPanel(AnimationFrame* frame, AnimationValueType type)
			: m_SelectedFrame(frame), m_FrameType(type)
		{
		}

		void OnImGuiRender();
		void SetContext(AnimationFrame* frame, AnimationValueType type) {
			m_SelectedFrame = frame;
			m_FrameType = type;
		}
	private:
		AnimationFrame* m_SelectedFrame = nullptr;
		AnimationValueType m_FrameType = AnimationValueType::MaxValue;
	};
}
