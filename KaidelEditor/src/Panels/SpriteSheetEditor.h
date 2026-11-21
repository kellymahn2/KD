#pragma once

#include <UI\UIHelper.h>
#include <Kaidel\Renderer\Sprite.h>

namespace Kaidel
{
	class SpriteSheetEditor
	{
	public:
		static void Open(Ref<SpriteSheet> sheet);
		static bool OnImGuiRender();
	private:
		static void DrawSpriteProperties();
		static void HandlePan();
		static void HandleZoom();
		static void HandleSpriteResize();
		static void HandleMove();
		static void HandleSelection();
		static void DrawImage();

	};
}
