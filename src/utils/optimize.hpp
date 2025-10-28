#ifndef CIRBO_SEARCH_UTILS_OPTIMIZE_HPP
#define CIRBO_SEARCH_UTILS_OPTIMIZE_HPP

// ===== COMPILER / PLATFORM =====
#if defined(_MSC_VER)
    #define CIRBO_OPT_CC_MSVC 1
#else
    #define CIRBO_OPT_CC_MSVC 0
#endif

#if defined(__clang__)
    #define CIRBO_OPT_CC_CLANG 1
#else
    #define CIRBO_OPT_CC_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
    #define CIRBO_OPT_CC_GCC 1
#else
    #define CIRBO_OPT_CC_GCC 0
#endif

#ifndef __has_attribute
    #define __has_attribute(x) 0
#endif
#ifndef __has_cpp_attribute
    #define __has_cpp_attribute(x) 0
#endif
#ifndef __has_builtin
    #define __has_builtin(x) 0
#endif

// ===== Inline / no inline =====
#if CIRBO_OPT_CC_MSVC
    #define CIRBO_OPT_FORCE_INLINE __forceinline
    #define CIRBO_OPT_NOINLINE __declspec(noinline)
#elif CIRBO_OPT_CC_GCC || CIRBO_OPT_CC_CLANG
    #define CIRBO_OPT_FORCE_INLINE inline __attribute__((always_inline))
    #define CIRBO_OPT_NOINLINE __attribute__((noinline))
#else
    #define CIRBO_OPT_FORCE_INLINE inline
    #define CIRBO_OPT_NOINLINE
#endif

// ===== Branch Prediction =====
#if __has_cpp_attribute(likely) && __has_cpp_attribute(unlikely)
    #define CIRBO_OPT_LIKELY(x) ((x)) [[likely]]
    #define CIRBO_OPT_UNLIKELY(x) ((x)) [[unlikely]]
#elif CIRBO_OPT_CC_GCC || CIRBO_OPT_CC_CLANG
    #define CIRBO_OPT_LIKELY(x) __builtin_expect(!!(x), 1)
    #define CIRBO_OPT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define CIRBO_OPT_LIKELY(x) (x)
    #define CIRBO_OPT_UNLIKELY(x) (x)
#endif

// ===== Assume / unreachable =====
#if CIRBO_OPT_CC_MSVC
    #define CIRBO_OPT_ASSUME(cond) __assume(cond)
#elif CIRBO_OPT_CC_CLANG && __has_builtin(__builtin_assume)
    #define CIRBO_OPT_ASSUME(cond) __builtin_assume(cond)
#elif CIRBO_OPT_CC_GCC && (__GNUC__ >= 13) && __has_builtin(__builtin_assume)
    #define CIRBO_OPT_ASSUME(cond) __builtin_assume(cond)
#else
    #define CIRBO_OPT_ASSUME(cond) \
        do {                       \
            if (!(cond)) {}        \
        } while (0)
#endif

#if CIRBO_OPT_CC_MSVC
    #define CIRBO_OPT_UNREACHABLE() __assume(0)
#elif CIRBO_OPT_CC_GCC || CIRBO_OPT_CC_CLANG
    #define CIRBO_OPT_UNREACHABLE() __builtin_unreachable()
#else
    #define CIRBO_OPT_UNREACHABLE() \
        do {                        \
        } while (0)
#endif

// ===== Hot/Cold =====
#if CIRBO_OPT_CC_GCC || CIRBO_OPT_CC_CLANG
    #define CIRBO_OPT_HOT __attribute__((hot))
    #define CIRBO_OPT_COLD __attribute__((cold))
#else
    #define CIRBO_OPT_HOT
    #define CIRBO_OPT_COLD
#endif

#endif  // CIRBO_SEARCH_UTILS_OPTIMIZE_HPP
