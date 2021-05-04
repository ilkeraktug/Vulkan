#pragma once
#include "Log.h"

#ifdef ENABLE_ASSERT
	#define VK_ASSERT(x, ...) { if(!(x)) { VK_ERROR("Assertion Failed : {0}", __VA_ARGS__); __debugbreak(); } }
	#define VK_CORE_ASSERT(x, ...) { if(!(x)) { VK_CORE_ERROR("Assertion Failed : {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define VK_ASSERT(x, ...) x
	#define VK_CORE_ASSERT(x, ...) x
#endif //ENABLE_ASSERT

#ifdef ENABLE_ASSERT
#define VK_CHECK(x) { if(!(x == 0)) { VK_ERROR("Assertion Failed : {0}", ##x); __debugbreak(); } }
#define VK_CORE_CHECK(x, ...) { if(!(x == 0)) { VK_CORE_ERROR("Assertion Failed : {0}", ##x); __debugbreak(); } }
#else
#define VK_CHECK(x) x
#define VK_CORE_CHECK(x, ...) x
#endif //ENABLE_ASSERT