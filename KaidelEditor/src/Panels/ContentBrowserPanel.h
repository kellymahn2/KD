#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "EditorContext.h"
#include <filesystem>
namespace Kaidel {
	class ContentBrowserPanel {
	public:
		static void OnImGuiRender();
	private:
		static void ShowFileNavigator();

		static void RenderThumbnail(const FileSystem::directory_entry& directoryEntry, uint32_t i);

		static void RenderListItem(const FileSystem::directory_entry& directoryEntry, uint32_t i);

		static void RenderCurrentDirectory();
	};
}
