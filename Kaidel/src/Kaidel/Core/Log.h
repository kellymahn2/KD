#pragma once

#include "Kaidel/Core/Base.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)


namespace Kaidel {

	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define KD_CORE_TRACE(...)    ::Kaidel::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KD_CORE_INFO(...)     ::Kaidel::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KD_CORE_WARN(...)     ::Kaidel::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KD_CORE_ERROR(...)    ::Kaidel::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KD_CORE_CRITICAL(...) ::Kaidel::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define KD_TRACE(...)         ::Kaidel::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KD_INFO(...)          ::Kaidel::Log::GetClientLogger()->info(__VA_ARGS__)
#define KD_WARN(...)          ::Kaidel::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KD_ERROR(...)         ::Kaidel::Log::GetClientLogger()->error(__VA_ARGS__)
#define KD_CRITICAL(...)      ::Kaidel::Log::GetClientLogger()->critical(__VA_ARGS__)
