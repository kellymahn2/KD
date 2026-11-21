#pragma once

#include "Kaidel/VisualShader/VisualShader.h"

namespace Kaidel {
	class VisualShaderNodeEditor {
	public:
		static void OnImGuiRender();

	private:
		static void DrawNodePicker(Ref<VisualShader> mat);
		static bool DrawNodeInternal(Ref<VisualShader> mat, VisualShaderNode& node, uint32_t& portID);
		static void DrawVisualShaderNode(uint32_t nodeID, Ref<VisualShader> mat, uint32_t& portID);

		static void DrawNodes(Ref<VisualShader> shader);

		static void HandleInteractions(Ref<VisualShader> shader);
	};
}
