/**
 * @file junco/log.hpp
 *
 * Defines junco's logging utilities, which allow for custom Loggers to be used
 * through a uniform LoggerTraits interface, reducing boilerplate.
 *
 * Junco's default logger (junco::Log) also allows the user to configure its
 * logging functions at runtime, letting the the user decide how messages are
 * recorded.
 */
#pragma once

#include <concepts>   // concepts
#include <format>     // std::format, std::format_string
#include <iostream>   // std::cout, std::cerr
#include <syncstream> // std::osyncstream

#if defined(JC_BUILD_DEBUG)
#define JC_ENABLE_LOGGING
#endif
#if defined(JC_BUILD_RELWITHDEBINFO)
#define JC_ENABLE_LOGGING
#endif
#if defined(JC_RELEASE)
#endif

namespace junco {
template <typename T>
concept Logger = requires(std::string msg) {
  { T::trace(msg) } -> std::same_as<void>;
  { T::standard(msg) } -> std::same_as<void>;
  { T::warning(msg) } -> std::same_as<void>;
  { T::error(msg) } -> std::same_as<void>;
  { T::fatal(msg) } -> std::same_as<void>;
};

/**
 * Provides a common interface for sending messages through a Logger object.
 * LoggerTraits handles formatting and inlining, so that log methods are only
 * included in source files when logging is enabled.
 */
template <Logger T> class LoggerTraits final {
public:
  template <typename... Args>
  inline static void trace(std::format_string<Args...> fmt,
                           Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = std::format(fmt, std::forward<Args>(args)...);
    T::trace(message);
#endif
  }
  template <typename... Args>
  inline static void standard(std::format_string<Args...> fmt,
                              Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = std::format(fmt, std::forward<Args>(args)...);
    T::standard(message);
#endif
  }
  template <typename... Args>
  inline static void warning(std::format_string<Args...> fmt,
                             Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = std::format(fmt, std::forward<Args>(args)...);
    T::warning(message);
#endif
  }
  template <typename... Args>
  inline static void error(std::format_string<Args...> fmt,
                           Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = std::format(fmt, std::forward<Args>(args)...);
    T::error(message);
#endif
  }
  template <typename... Args>
  inline static void fatal(std::format_string<Args...> fmt,
                           Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = std::format(fmt, std::forward<Args>(args)...);
    T::fatal(message);
#endif
  }
};

/**
 * All functions that can be overwritten for junco's default logger.
 * @note When overwriting log functions, be sure to account for parallel access.
 * Use std::osyncstream or other for thread safety.
 */
struct LogFunctions {
  using LogFunction = void (*)(const std::string &);
  LogFunction trace;
  LogFunction standard;
  LogFunction warning;
  LogFunction error;
  LogFunction fatal;
  // Overwrites all log channels with the given function. Note that, if defined,
  // this is the ONLY function that will be used for logging (all other
  // overwritten functions are ignored).
  LogFunction all;
};

/**
 * Implementation for junco's default logger. Uses thread-safe logging methods
 * by default, which can be overwritten with user-defined functions.
 */
class StandardLogger final {
public:
  static void trace(const std::string &msg) noexcept {
    if (functions.all)
      functions.all(msg);
    else if (functions.trace)
      functions.trace(msg);
    else
      default_trace(msg);
  }
  static void standard(const std::string &msg) noexcept {
    if (functions.all)
      functions.all(msg);
    else if (functions.standard)
      functions.standard(msg);
    else
      default_standard(msg);
  }
  static void warning(const std::string &msg) noexcept {
    if (functions.all)
      functions.all(msg);
    else if (functions.warning)
      functions.warning(msg);
    else
      default_warning(msg);
  }
  static void error(const std::string &msg) noexcept {
    if (functions.all)
      functions.all(msg);
    else if (functions.error)
      functions.error(msg);
    else
      default_error(msg);
  }
  static void fatal(const std::string &msg) noexcept {
    if (functions.all)
      functions.all(msg);
    else if (functions.fatal)
      functions.fatal(msg);
    else
      default_fatal(msg);
  }

  static void set_log_functions(const LogFunctions &new_functions) noexcept {
    functions = new_functions;
  }

private:
  static void default_trace(const std::string &msg) noexcept {
    std::osyncstream(std::cout) << msg << std::endl;
  }
  static void default_standard(const std::string &msg) noexcept {
    std::osyncstream(std::cout) << msg << std::endl;
  }
  static void default_warning(const std::string &msg) noexcept {
    std::osyncstream(std::cerr) << "[warning]" << msg << std::endl;
  }
  static void default_error(const std::string &msg) noexcept {
    std::osyncstream(std::cerr) << "[error]" << msg << std::endl;
  }
  static void default_fatal(const std::string &msg) noexcept {
    std::osyncstream(std::cerr) << "[fatal]" << msg << std::endl;
  }

  inline static LogFunctions functions{};
};

using Log = LoggerTraits<StandardLogger>;
} // namespace junco