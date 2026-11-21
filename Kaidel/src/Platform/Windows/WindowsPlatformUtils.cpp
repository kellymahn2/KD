#include "KDpch.h"
#include "Kaidel/Utils/PlatformUtils.h"
#include "Kaidel/Core/Application.h"

#include <commdlg.h>
#include <shobjidl.h>
#include <combaseapi.h>
#include <GLFW/glfw3.h>
#include <shlobj.h>
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


	std::optional<std::string> FileDialogs::SelectFolder()
	{
		IFileOpenDialog* dialog = nullptr;

		HRESULT hr = CoCreateInstance(
			CLSID_FileOpenDialog,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&dialog)
		);

		if (FAILED(hr))
			return std::nullopt;

		DWORD options = 0;
		dialog->GetOptions(&options);
		dialog->SetOptions(options | FOS_PICKFOLDERS);

		HWND window = glfwGetWin32Window(
			(GLFWwindow*)Application::Get()
			.GetWindow()
			.GetNativeWindow()
		);

		hr = dialog->Show(window);

		if (FAILED(hr))
		{
			dialog->Release();
			return std::nullopt;
		}

		IShellItem* item = nullptr;
		hr = dialog->GetResult(&item);

		if (FAILED(hr))
		{
			dialog->Release();
			return std::nullopt;
		}

		PWSTR path = nullptr;
		hr = item->GetDisplayName(SIGDN_FILESYSPATH, &path);

		std::optional<std::string> result;

		if (SUCCEEDED(hr))
		{
			int size = WideCharToMultiByte(
				CP_UTF8,
				0,
				path,
				-1,
				nullptr,
				0,
				nullptr,
				nullptr
			);

			std::string utf8(size - 1, '\0');

			WideCharToMultiByte(
				CP_UTF8,
				0,
				path,
				-1,
				utf8.data(),
				size,
				nullptr,
				nullptr
			);

			result = utf8;

			CoTaskMemFree(path);
		}

		item->Release();
		dialog->Release();

		return result;
	}

	using FileWatcherType = filewatch::FileWatch<std::string>;

	FileWatcher::FileWatcher(const std::string& path, const FileWatcherHandlers& handlers)
	{
		Reset(path, handlers);
	}

	void FileWatcher::Reset(const std::string& path, const FileWatcherHandlers& handlers)
	{
		m_Handlers = handlers;
		m_Watching = path;

		m_Watcher = new FileWatcherType(path,
			[this, path = Path(path)](const std::string& file, const filewatch::Event event)
			{
				if (FileSystem::is_directory(path / file))
				{
					return;
				}

				//Skip metadata files
				if (Path(path / file).extension() == ".mtd")
				{
					return;
				}

				{
					m_Mutex.lock_shared();
					auto it = m_Handlers.Consumes.find(path / file);
					
					if (it != m_Handlers.Consumes.end())
					{
						--it->second;
						KD_CORE_INFO("{}, {}, Consumed, remaining {}", file, filewatch::event_to_string(event), it->second);

						if (it->second == 0)
						{
							m_Mutex.unlock_shared();
							m_Mutex.lock();
							if (it->second == 0)
							{
								m_Handlers.Consumes.erase(it);
							}
							m_Mutex.unlock();
							return;
						}	
						m_Mutex.unlock_shared();
						return;
					}
					m_Mutex.unlock_shared();
				}

				KD_CORE_INFO("{}, {}", file, filewatch::event_to_string(event));
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
