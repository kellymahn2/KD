#pragma once


#include "Kaidel/Core/PlatformDetection.h"
#include "Kaidel/Core/Random.h"
#include "Kaidel/Core/IRCP.h"
#include <memory>
#include <filesystem>

#pragma warning(disable: 4996)
#pragma warning(disable: 4005)
#pragma warning(disable: 4002)


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

namespace Kaidel {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = IRCPointer<T,false>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return CreateIRCPointer<T,false>(std::forward<Args>(args)...);
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
	
	namespace FileSystem = std::filesystem;


	using Path = FileSystem::path;
	using DirectoryEntry = FileSystem::directory_entry;


}

#include "Kaidel/Core/Log.h"
#include "Kaidel/Core/Assert.h"
