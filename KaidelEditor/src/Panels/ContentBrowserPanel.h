#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "PanelContext.h"
#include <filesystem>
namespace Kaidel {
	class ContentBrowserPanel {
	public:
		struct Icons {
			Ref<Texture2D> DirectoryIcon;
			Ref<Texture2D> FileIcon;
		};
		ContentBrowserPanel();

		void SetContext(Ref<PanelContext> context) {
			m_Context = context;
		}

		void SetCurrentPath(const FileSystem::path& path) { m_CurrentPath = path; }
		void SetStartPath(const FileSystem::path& path) { m_StartPath= path; }
		void OnImGuiRender();


	private:
		Ref<PanelContext> m_Context;
		void ShowFileNavigator();

		void RenderThumbnail(const FileSystem::directory_entry& directoryEntry,uint32_t i);

		std::filesystem::path m_StartPath;
		std::filesystem::path m_CurrentPath;
		char m_SelectedFileName[100] = {0};
		bool m_WasNew = false;
		Icons m_Icons;
	};
}
