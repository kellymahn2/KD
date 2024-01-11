#pragma once

#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/Console.h"
// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)



class Log
{
public:
	static void Init();

	static Kaidel::Ref<Kaidel::Console>& GetCoreLogger() { return s_CoreLogger; }
	static Kaidel::Ref<Kaidel::Console>& GetClientLogger() { return s_ClientLogger; }
	class CoreConsole : public Kaidel::Console{
	public:
		template<typename... Args>
		void Trace(const std::string& str,Args&&... args) {
			Kaidel::Console::Log(fmt::format(str, std::forward<Args>(args)...),Kaidel::MessageLevel::Log);
		}
		template<typename... Args>
		void Info(const std::string& str, Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str, std::forward<Args>(args)...), Kaidel::MessageLevel::Info);
		}
		template<typename... Args>
		void Warn(const std::string& str, Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Warn);
		}
		template<typename... Args>
		void Error(const std::string& str="", Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Error);
		}
		template<typename... Args>
		void Critical(const std::string& str,Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Error);
		}
	private:
	};
	class ClientConsole :public Kaidel::Console {
	public:
		template<typename... Args>
		void Trace(const std::string& str,Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Log);
		}
		template<typename... Args>
		void Info(const std::string& str,Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Info);
		}
		template<typename... Args>
		void Warn(const std::string& str,Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Warn);
		}
		template<typename... Args>
		void Error(const std::string& str="", Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Error);
		}
		template<typename... Args>
		void Critical(const std::string& str,Args&&... args)
		{
			Kaidel::Console::Log(fmt::format(str,std::forward<Args>(args)...),Kaidel::MessageLevel::Error);
		}
	private:
	};
private:
	static Kaidel::Ref<Kaidel::Console> s_CoreLogger;
	static Kaidel::Ref<Kaidel::Console> s_ClientLogger;
};


// Core log macros
#ifdef KD_DEBUG
#define KD_CORE_TRACE(...)    ((::Log::CoreConsole*)::Log::GetCoreLogger().get())->Trace(__VA_ARGS__)
#define KD_CORE_INFO(...)     ((::Log::CoreConsole*)::Log::GetCoreLogger().get())->Info(__VA_ARGS__)
#define KD_CORE_WARN(...)     ((::Log::CoreConsole*)::Log::GetCoreLogger().get())->Warn(__VA_ARGS__)
#define KD_CORE_ERROR(...)    ((::Log::CoreConsole*)::Log::GetCoreLogger().get())->Error(__VA_ARGS__)
#define KD_CORE_CRITICAL(...) ((::Log::CoreConsole*)::Log::GetCoreLogger().get())->Critical(__VA_ARGS__)
#else
#define KD_CORE_TRACE(...)    
#define KD_CORE_INFO(...)     
#define KD_CORE_WARN(...)     
#define KD_CORE_ERROR(...)    
#define KD_CORE_CRITICAL(...) 
#endif
// Client log macros
#define KD_TRACE(...)         ((::Log::ClientConsole*)::Log::GetClientLogger().get())->Trace(__VA_ARGS__)
#define KD_INFO(...)          ((::Log::ClientConsole*)::Log::GetClientLogger().get())->Info(__VA_ARGS__)
#define KD_WARN(...)          ((::Log::ClientConsole*)::Log::GetClientLogger().get())->Warn(__VA_ARGS__)
#define KD_ERROR(...)         ((::Log::ClientConsole*)::Log::GetClientLogger().get())->Error(__VA_ARGS__)
#define KD_CRITICAL(...)      ((::Log::ClientConsole*)::Log::GetClientLogger().get())->Critical(__VA_ARGS__)
