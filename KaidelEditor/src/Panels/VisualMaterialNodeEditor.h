#pragma once

#include "Kaidel/VisualMaterial/VisualMaterial.h"

namespace Kaidel {
	class VisualMaterialNodeEditor {
	public:
		static void OnImGuiRender(Ref<VisualMaterial> material);

	private:
		static void DrawNodePicker(Ref<VisualMaterial> mat);
		static void DrawNodeInternal(Ref<VisualMaterial> mat, VisualMaterialNode& node, uint32_t& portID);
		static void DrawVisualMaterialNode(uint32_t nodeID, Ref<VisualMaterial> mat, uint32_t& portID);
	};
}
