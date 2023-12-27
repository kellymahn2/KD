#pragma once

#include "Kaidel/Core/Base.h"
#include "Kaidel/Core/Log.h"
#include <filesystem>

#ifdef KD_ENABLE_ASSERTS

	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define KD_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { KD##type##ERROR(msg, __VA_ARGS__); KD_DEBUGBREAK(); } }
	#define KD_INTERNAL_ASSERT_WITH_MSG(type, check, ...) KD_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define KD_INTERNAL_ASSERT_NO_MSG(type, check) KD_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", KD_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define KD_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define KD_INTERNAL_ASSERT_GET_MACRO(...) KD_EXPAND_MACRO( KD_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, KD_INTERNAL_ASSERT_WITH_MSG, KD_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	/*#define KD_ASSERT(...) KD_EXPAND_MACRO( KD_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define KD_CORE_ASSERT(...) KD_EXPAND_MACRO( KD_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )*/
	#define KD_ASSERT(check,...) {if(!(check)){KD_ERROR(__VA_ARGS__);__debugbreak();}}
	#define KD_CORE_ASSERT(check,...) {if(!(check)){KD_CORE_ERROR(__VA_ARGS__);__debugbreak();}}
#else
	#define KD_ASSERT(...)
	#define KD_CORE_ASSERT(...)
#endif
