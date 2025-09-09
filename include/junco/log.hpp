/**
 * @file junco/log.hpp
 *
 * Defines junco's logging utilities, which allow for basic log channels,
 * formatting, and stream redirection.
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

template <Logger T> class LoggerTraits final {
public:
  template <typename... Args>
  inline static void trace(std::format_string<Args...> fmt,
                           Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = format_with_args(fmt, args...);
    T::trace(message);
#endif
  }
  template <typename... Args>
  inline static void standard(std::format_string<Args...> fmt,
                              Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = format_with_args(fmt, args...);
    T::standard(message);
#endif
  }
  template <typename... Args>
  inline static void warning(std::format_string<Args...> fmt,
                             Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = format_with_args(fmt, args...);
    T::warning(message);
#endif
  }
  template <typename... Args>
  inline static void error(std::format_string<Args...> fmt,
                           Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = format_with_args(fmt, args...);
    T::error(message);
#endif
  }
  template <typename... Args>
  inline static void fatal(std::format_string<Args...> fmt,
                           Args &&...args) noexcept {
#ifdef JC_ENABLE_LOGGING
    auto message = format_with_args(fmt, args...);
    T::fatal(message);
#endif
  }

private:
  template <typename... Args>
  inline static std::string format_with_args(std::format_string<Args...> fmt,
                                             Args &&...args) noexcept {
    return std::format(fmt, args...);
  }
};

struct LogFunctions {
  using LogFunction = void (*)(const std::string &);
  LogFunction trace;
  LogFunction standard;
  LogFunction warning;
  LogFunction error;
  LogFunction fatal;
  LogFunction all;
};

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
    std::osyncstream(std::cerr) << msg << std::endl;
  }
  static void default_error(const std::string &msg) noexcept {
    std::osyncstream(std::cerr) << msg << std::endl;
  }
  static void default_fatal(const std::string &msg) noexcept {
    std::osyncstream(std::cerr) << msg << std::endl;
  }

  inline static LogFunctions functions{};
};

using Log = LoggerTraits<StandardLogger>;
} // namespace junco