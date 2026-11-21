#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE 1

#include "Kaidel/Core/PlatformDetection.h"
#include "Kaidel/Core/Random.h"
#include "Kaidel/Core/IRCP.h"
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <spdlog/fmt/fmt.h>

#include <fstream>

#pragma warning(disable: 4996)
#pragma warning(disable: 4005)
#pragma warning(disable: 4002)
#pragma warning(disable: 4828)


#ifdef KD_DEBUG
#if defined(KD_PLATFORM_WINDOWS)
#define KD_DEBUGBREAK() __debugbreak()
#elif defined(KD_PLATFORM_LINUX)
#include <signal.h>
#define KD_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define KD_ENABLE_ASSERTS
#else
#define KD_DEBUGBREAK()
#endif

#define KD_EXPAND_MACRO(x) x
#define KD_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define KD_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define ALLOCA(size) ((size != 0) ? alloca(size) : nullptr)
#define ALLOCA_ARRAY(type, count) ((type*)ALLOCA(sizeof(type) * (count)))
#define ALLOCA_SINGLE(type) ALLOCA_ARRAY(type, 1)


template <>
struct fmt::formatter<std::filesystem::path> : fmt::formatter<std::string> {
    auto format(const std::filesystem::path& p, format_context& ctx) const {
        return fmt::formatter<std::string>::format(p.string(), ctx);
    }
};

namespace Kaidel {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = IRCPointer<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return CreateIRCPointer<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	constexpr Ref<T> PlaceRef(Ref<T> ptr, Args&&... args)
	{
		uint64_t count = ptr->Count;
		new (ptr.Get()) T(std::forward<Args>(args)...);
		ptr->Count = count;
		return ptr;
	}

	template<typename T, uint32_t _Count>
	constexpr std::array<T, _Count> FillArray(const T& val)
	{
		std::array<T, _Count> array;
		array.fill(val);
		return array;
	}

	template<typename T>
	constexpr T Fact(T n) {
		if (n == static_cast<T>(0))
			return static_cast<T>(1);
		if (n == static_cast<T>(1))
			return static_cast<T>(1);
		if (n == static_cast<T>(2))
			return static_cast<T>(2);
		return n * Fact<T>(n - 1);
	}
	template<typename  T>
	T CalcBinomialCoefficient(T n, T r) {
		return (Fact(n)) / (Fact(r) * Fact(n - r));

	}

	template<typename T>
	struct AlignedBuffer {
		alignas(T) uint8_t Ptr[sizeof(T)];
	};

	template<typename T>
	T* CreateUninitialized() 
	{
		

		T* ptr = (T*)new AlignedBuffer<T>;
		memset(ptr, 0, sizeof(AlignedBuffer<T>));

		return ptr;
	}

	namespace FileSystem = std::filesystem;


	using Path = FileSystem::path;
	using DirectoryEntry = FileSystem::directory_entry;

	typedef uintptr_t RendererID;

	template<typename T>
	class TLS {
	public:
		auto& GetMap() { return m_Map; }
		const auto& GetMap() const { return m_Map; }

		T& operator*() { return m_Map[std::this_thread::get_id()]; }
		const T& operator*() const { return m_Map.at(std::this_thread::get_id()); }
	private:
		std::unordered_map<std::thread::id, T> m_Map;
	};

	static bool GainAccessToFile(std::ifstream* outFile, const Path& path, std::ios_base::openmode openMode = std::ios::in, 
		const std::chrono::milliseconds& sleepTime = std::chrono::milliseconds(200))
	{
		bool good = false;

		while (FileSystem::exists(path))
		{
			std::ifstream file(path, openMode);

			if (file.is_open())
			{
				if(outFile)
					outFile->swap(file);
				good = true;
				break;
			}

			std::this_thread::sleep_for(sleepTime);
		}

		return good;
	}

	static bool GainAccessToFile(FILE** outFile, const std::string& path, const char* openMode = "rt",
		const std::chrono::milliseconds& sleepTime = std::chrono::milliseconds(200))
	{
		bool good = false;

		while (FileSystem::exists(path))
		{
			if (fopen_s(outFile, path.c_str(), openMode) == 0)
			{
				good = true;
				break;
			}

			std::this_thread::sleep_for(sleepTime);
		}

		return good;
	}

}

#include "Kaidel/Core/Log.h"
#include "Kaidel/Core/Assert.h"
