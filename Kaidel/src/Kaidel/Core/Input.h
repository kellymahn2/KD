#pragma once

#include <glm/glm.hpp>

#include "Kaidel/Core/KeyCodes.h"
#include "Kaidel/Core/MouseCodes.h"

namespace Kaidel {

	class Input
	{
	public:
		static bool IsKeyDown(KeyCode key);
		static bool IsMouseButtonDown(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	protected:
		static bool s_HeldKeysMap[348];
		static bool s_HeldButtonsMap[7];
	};
}
