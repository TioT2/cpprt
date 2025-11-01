/** Main definition file */

#ifndef RT_DEF_H_
#define RT_DEF_H_

#include <atomic>
#include <cstring>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <span>
#include <thread>

/// Project namespace
namespace rt {
    /// Debug mode flag
    constexpr bool IS_DEBUG =
        #ifdef NDEBUG
            false
        #else
            true
        #endif
        ;
}

#endif // !defined(RT_DEF_H_)

// rt_def.h
