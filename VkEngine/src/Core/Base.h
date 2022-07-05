#pragma once

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

// Logging and Assertions
#ifdef VKE_DEBUG
	#define CORE_INFO(...) spdlog::info(__VA_ARGS__);
	#define CORE_WARN(...) spdlog::warn(__VA_ARGS__);
	#define CORE_ERROR(...) spdlog::error(__VA_ARGS__);

	#define CORE_ASSERT(condition, msg) { if(!(condition)) { CORE_ERROR(msg); __debugbreak(); } }
	#define VK_CHECK(condition, msg) { if(condition != VK_SUCCESS) { CORE_ERROR(msg); __debugbreak(); } }
#else
	#define CORE_INFO(msg);
	#define CORE_WARN(msg);
	#define CORE_ERROR(msg);

	#define CORE_ASSERT(condition, msg)
	#define VK_CHECK(condition, msg)
#endif