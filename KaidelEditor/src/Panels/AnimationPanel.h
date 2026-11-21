#pragma once
#include "Kaidel/Scene/Entity.h"
#include "EditorContext.h"

namespace Kaidel {
	class AnimationPanel {
	public:
		static void OnImGuiRender();

		static void OnPositionChanged();

		static void OnRotationChanged();

		static void OnScaleChanged();

		static void OnSelectionChanged();

	private:
		static void SetupPlot(bool header);
		static void HeaderPlot();
		static void ShowPanel(Ref<AnimationTree> tree);
		static void RenderTrack(AnimationTrack& track);
		static void RenderNode(AnimationTree::AnimationTreeNode& currNode, const std::string& name);
		
	};
}
