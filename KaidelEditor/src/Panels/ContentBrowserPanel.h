#pragma once
#include <filesystem>
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
namespace Kaidel {
	
	class ContentBrowserPanel {
	public:
		struct Icons {
			Ref<Texture2D> DirectoryIcon;
			Ref<Texture2D> FileIcon;
		};
		ContentBrowserPanel();
		void OnImGuiRender();
	private:
		void ShowFileNavigator();
		std::filesystem::path m_CurrentPath;
		char m_SelectedFileName[24] = {0};
		Icons m_Icons;
	};
}
