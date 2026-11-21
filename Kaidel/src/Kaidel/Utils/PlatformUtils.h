#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <shared_mutex>

namespace Kaidel {

	class FileDialogs
	{
	public:
		// These return empty strings if canceled
		static std::optional<std::string> OpenFile(const char* filter);
		static std::optional<std::string> SaveFile(const char* filter);
		static std::optional<std::string> SelectFolder();
	};

	struct FileWatcherHandlers
	{
		std::function<void(const std::string&)> OnAdded;
		std::function<void(const std::string&)> OnRemoved;
		std::function<void(const std::string&)> OnModified;
		std::function<void(const std::string&)> OnRenamedOld;
		std::function<void(const std::string&)> OnRenamedNew;

		std::unordered_map<Path, uint32_t> Consumes;
	};


	class FileWatcher {
	public:
		FileWatcher() = default;
		FileWatcher(const std::string& path, const FileWatcherHandlers& handlers);
		~FileWatcher();
		void Reset(const std::string& path, const FileWatcherHandlers& handlers);

		void Consume(const Path& path, uint32_t consumeAmount)
		{
			m_Mutex.lock();
			m_Handlers.Consumes[path] += consumeAmount;
			m_Mutex.unlock();
		}

		const Path& GetWatchingPath() const { return m_Watching; }
	private:
		void* m_Watcher = nullptr;
		std::shared_mutex m_Mutex;
		FileWatcherHandlers m_Handlers;
		Path m_Watching;
	};

}
