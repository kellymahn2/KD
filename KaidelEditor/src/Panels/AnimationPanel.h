#pragma once
#include "Kaidel/Scene/Entity.h"
#include "EditorContext.h"

namespace Kaidel {
	class AnimationPanel {
	public:
		void OnImGuiRender();

		void SetContext(Ref<AnimationTree> context) { m_Context = context; }

	private:
		void HeaderPlot();
		void ShowPanel(Ref<AnimationTree> tree);
		void RenderTrack(AnimationTrack& track);
		void RenderNode(AnimationTree::AnimationTreeNode& currNode, const std::string& name);
	private:
		Ref<AnimationTree> m_Context;
		double m_LimitMinX = 0.0, m_LimitMaxX = 2.0;
		uint32_t m_Count = 1;
	};
}
