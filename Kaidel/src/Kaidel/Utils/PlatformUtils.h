#pragma once

#include <string>
#include <optional>

namespace Kaidel {

	class FileDialogs
	{
	public:
		// These return empty strings if canceled
		static std::optional<std::string> OpenFile(const char* filter);
		static std::optional<std::string> SaveFile(const char* filter);
		static std::optional<std::string> OpenFolder(bool empty);
	};

	struct FileWatcherHandlers
	{
		std::function<void(const std::string&)> OnAdded;
		std::function<void(const std::string&)> OnRemoved;
		std::function<void(const std::string&)> OnModified;
		std::function<void(const std::string&)> OnRenamedOld;
		std::function<void(const std::string&)> OnRenamedNew;
	};


	class FileWatcher {
	public:
		FileWatcher() = default;
		FileWatcher(const std::string& path, const FileWatcherHandlers& handlers);
		~FileWatcher();
		void Reset(const std::string& path, const FileWatcherHandlers& handlers);
	private:
		void* m_Watcher = nullptr;
		FileWatcherHandlers m_Handlers;
	};

}
