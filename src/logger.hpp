#ifndef CIRBO_SEARCH_LOGGER_HPP
#define CIRBO_SEARCH_LOGGER_HPP

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <string>
#include <utility>

#include "utils/optimize.hpp"

namespace cirbo::log
{

// Enumerates available log levels.
enum class LogLevel : std::uint8_t
{
    DEBUG   = 10,
    INFO    = 20,
    WARNING = 30,
    ERROR   = 40,
    SILENT  = UINT8_MAX
};

// Global logging level
#ifdef CIRBO_ENABLE_DEBUG_LOGGING
static constexpr auto CompileLogLevel = LogLevel::DEBUG;
#else
static constexpr auto CompileLogLevel = LogLevel::INFO;
#endif

// Basic logging to std::cout.
CIRBO_OPT_FORCE_INLINE void LOG_OUT() { std::cout << std::endl; }

// Basic logging to std::cout.
template<class T, class... Args>
CIRBO_OPT_FORCE_INLINE void LOG_OUT(T const& t, Args const&... args)
{
    std::cout << t;
    LOG_OUT(args...);
}

// Basic logging to std::cerr.
CIRBO_OPT_FORCE_INLINE void LOG_ERR() { std::cerr << std::endl; }

// Basic logging to std::cerr.
template<class T, class... Args>
CIRBO_OPT_FORCE_INLINE void LOG_ERR(T const& t, Args const&... args)
{
    std::cerr << t;
    LOG_ERR(args...);
}

/**
 * Named logger that allows one to write variadic argument messages to stderr and stdout.
 */
class LoggerImpl
{
public:
    std::string name;

    LoggerImpl() noexcept
        : name("Logger")
    {
    }

    explicit LoggerImpl(std::string const& _name) noexcept
        : name(_name)
    {
    }

    explicit LoggerImpl(std::string&& _name) noexcept
        : name(std::move(_name))
    {
    }

    ~LoggerImpl() = default;

    void setName(std::string const& new_name) { name = new_name; }

    template<class T, class... Args>
    CIRBO_OPT_FORCE_INLINE void debug(T const& t, Args const&... args) const
    {
        if constexpr (LogLevel::DEBUG >= CompileLogLevel)
        {
            LOG_ERR("<", name, ">(", _getCurrentTime(), ") DEBUG: ", t, args...);
        }
    }

    template<class T, class... Args>
    CIRBO_OPT_FORCE_INLINE void info(T const& t, Args const&... args) const
    {
        if constexpr (LogLevel::INFO >= CompileLogLevel)
        {
            LOG_ERR("<", name, ">(", _getCurrentTime(), ") INFO: ", t, args...);
        }
    }

    template<class T, class... Args>
    CIRBO_OPT_FORCE_INLINE void warn(T const& t, Args const&... args) const
    {
        if constexpr (LogLevel::WARNING >= CompileLogLevel)
        {
            LOG_ERR("<", name, ">(", _getCurrentTime(), ") WARNING: ", t, args...);
        }
    }

    template<class T, class... Args>
    CIRBO_OPT_FORCE_INLINE void error(T const& t, Args const&... args) const
    {
        if constexpr (LogLevel::ERROR >= CompileLogLevel)
        {
            LOG_ERR("<", name, ">(", _getCurrentTime(), ") ERROR: ", t, args...);
        }
    }

private:
    static CIRBO_OPT_FORCE_INLINE std::string _getCurrentTime(const char* fmt = "%Y-%m-%d %H:%M:%S")
    {
        using clock = std::chrono::system_clock;
        const auto now = clock::now();
        const std::time_t t = clock::to_time_t(now);

        std::tm tm{};
        #if defined(_WIN32)
        localtime_s(&tm, &t);   // MSVC/Windows
        #else
        localtime_r(&t, &tm);   // POSIX
        #endif

        constexpr size_t CURRENT_TIME_BUFFER_SIZE = 64;
        char buf[CURRENT_TIME_BUFFER_SIZE];
        const std::size_t n = std::strftime(buf, sizeof(buf), fmt, &tm);
        return {buf, n};
    }

};

/**
 * Global instance of logger, which is used as logger in free functions call (`debug`, `info`, `warn` and `error`).
 */
static CIRBO_OPT_FORCE_INLINE LoggerImpl& getGlobalLogger()
{
    static LoggerImpl instance("CIRBO-SEARCH");
    return instance;
}

static CIRBO_OPT_FORCE_INLINE void setGlobalLoggerName(std::string const& name)
{
    return getGlobalLogger().setName(name);
}

template<class... Args>
CIRBO_OPT_FORCE_INLINE void debug(Args const&... args)
{
    getGlobalLogger().debug(args...);
}

template<class... Args>
CIRBO_OPT_FORCE_INLINE void info(Args const&... args)
{
    getGlobalLogger().info(args...);
}

template<class... Args>
CIRBO_OPT_FORCE_INLINE void warn(Args const&... args)
{
    getGlobalLogger().warn(args...);
}

template<class... Args>
CIRBO_OPT_FORCE_INLINE void error(Args const&... args)
{
    getGlobalLogger().error(args...);
}

}  // namespace cirbo::log

#endif  // CIRBO_SEARCH_LOGGER_HPP
