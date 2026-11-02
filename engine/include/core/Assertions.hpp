//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "core/Log.hpp"

#if defined(_MSC_VER)
#define ENGINE_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
#if __has_builtin(__builtin_debugtrap)
#define ENGINE_DEBUGBREAK() __builtin_debugtrap()
#else
#define ENGINE_DEBUGBREAK() __builtin_trap()
#endif
#else
#include <csignal>
#define ENGINE_DEBUGBREAK() std::raise(SIGTRAP)
#endif

#if !defined(NDEBUG) || defined(_DEBUG)
#define ENGINE_ENABLE_ASSERTS 1
#endif

#if defined(ENGINE_ENABLE_ASSERTS)
#define CORE_ASSERT(condition, ...) \
        do { \
            if (!(condition)) { \
                CORE_ERROR("Assertion failed: {} [{}:{}]", #condition, __FILE__, __LINE__); \
                CORE_ERROR("Details: " __VA_ARGS__); \
                ENGINE_DEBUGBREAK(); \
            } \
        } while (false)
#else
#define CORE_ASSERT(condition, ...) ((void)0)
#endif
