#include "KDpch.h"
#include "Kaidel/Utils/PlatformUtils.h"
#include "Kaidel/Core/Application.h"

#include <commdlg.h>
#include <shobjidl.h>
#include <combaseapi.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <FileWatch.h>

namespace Kaidel {

	std::optional<std::string> FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		return std::nullopt;
	}

	std::optional<std::string> FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;
		return std::nullopt;
	}


	std::optional<std::string> FileDialogs::OpenFolder(bool empty)
	{
		/*IFileOpenDialog* fileOpen;
		CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileOpen));


		IShellItem* item;

		fileOpen->GetResult(&item);

		PWSTR path;

		if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
		{
			CoTaskMemFree(pszStringPath);
			pItem->Release();
			pFileOpen->Release();
		}*/

		return {};
	}

	using FileWatcherType = filewatch::FileWatch<std::string>;

	FileWatcher::FileWatcher(const std::string& path, const FileWatcherHandlers& handlers)
	{
		Reset(path, handlers);
	}

	void FileWatcher::Reset(const std::string& path, const FileWatcherHandlers& handlers)
	{
		m_Handlers = handlers;
		m_Watcher = new FileWatcherType(path,
			[this](const std::string& file, const filewatch::Event event)
			{
				KD_CORE_INFO("{}, {}", file, (uint32_t)event);
				switch (event)
				{
				case filewatch::Event::added: m_Handlers.OnAdded(file); break;
				case filewatch::Event::removed: m_Handlers.OnRemoved(file); break;
				case filewatch::Event::modified: m_Handlers.OnModified(file); break;
				case filewatch::Event::renamed_old: m_Handlers.OnRenamedOld(file); break;
				case filewatch::Event::renamed_new: m_Handlers.OnRenamedNew(file); break;
				}
			});
	}

	FileWatcher::~FileWatcher()
	{
		delete m_Watcher;
	}

}
