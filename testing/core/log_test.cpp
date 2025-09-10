#include "junco/log.hpp"
#include <format>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <thread>

// Include this to send a message through each log stream to demonstrate their
// appearance
// #define DO_LOG_DEMO

/**
 * Custom logger specialized for comparing the result of a log operation to an
 * expected value.
 */
class ComparisonLogger final {
public:
  static void trace(const std::string &msg) noexcept { check_message(msg); }
  static void standard(const std::string &msg) noexcept { check_message(msg); }
  static void warning(const std::string &msg) noexcept { check_message(msg); }
  static void error(const std::string &msg) noexcept { check_message(msg); }
  static void fatal(const std::string &msg) noexcept { check_message(msg); }
  static void set_expected(const std::string &val) noexcept { expected = val; }
  static bool did_match() noexcept { return matched; }

private:
  static void check_message(const std::string &msg) {
    // std::cout << expected << " : " << msg << std::endl;
    matched = msg == expected;
  }
  inline static std::string expected;
  inline static bool matched;
};
using CompareLog = junco::LoggerTraits<ComparisonLogger>;

/**
 * Tests the custom logger with a set of basic string messages.
 */
TEST(LogTesting, CustomStrings) {
  const auto &messages = {"this is a message!", "this is another message!",
                          "this is a third message!",
                          "this is the final message!"};
  for (const auto &msg : messages) {
    ComparisonLogger::set_expected(msg);
    CompareLog::standard("{}", msg);
    EXPECT_TRUE(ComparisonLogger::did_match());
  }
}

/**
 * Tests the custom logger with a series of formatted messages.
 */
TEST(LogTesting, CustomFormat) {
  for (int i = 0; i < 10; ++i) {
    auto expected = "this is the " + std::to_string(i) + " message!";
    ComparisonLogger::set_expected(expected);
    CompareLog::standard("this is the {} message!", i);
    EXPECT_TRUE(ComparisonLogger::did_match());
  }
}

TEST(LogTesting, CustomMultipleArgs) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      for (int k = 0; k < 4; ++k) {
        auto expected = std::format("{}, {}, {}", i, j, k);
        ComparisonLogger::set_expected(expected);
        CompareLog::standard("{}, {}, {}", i, j, k);
        EXPECT_TRUE(ComparisonLogger::did_match());
      }
    }
  }
}

/**
 * Tests junco's default logger and its ability to redirect log functions.
 */
TEST(LogTesting, DefaultFunctions) {
  auto functions = junco::LogFunctions{.all = ComparisonLogger::standard};
  junco::StandardLogger::set_log_functions(functions);
  ComparisonLogger::set_expected(
      "this is a message sent through the comparison logger!");
  junco::Log::standard("this is a message sent through the comparison logger!");
  EXPECT_TRUE(ComparisonLogger::did_match());
  junco::Log::warning("this is a message sent through the comparison logger!");
  EXPECT_TRUE(ComparisonLogger::did_match());
  junco::Log::error("this is a message sent through the comparison logger!");
  EXPECT_TRUE(ComparisonLogger::did_match());
  // Reset functions
  junco::StandardLogger::set_log_functions(junco::LogFunctions{});
}

/**
 * Ensures that junco's standard logger is thread safe.
 */
TEST(LogTesting, DefaultThreadSafety) {
  // Redirect cout to string stream
  auto output_strm = std::stringstream{};
  auto old_rdbuf = static_cast<std::streambuf *>(std::cout.rdbuf());
  std::cout.rdbuf(output_strm.rdbuf());

  // Send a batch of messages among several threads
  auto messages = {
      "This is the first thread!",
      "This is the second thread!",
      "This is the third thread!",
      "This is the fourth thread!",
  };
  auto print_message = [](const char *msg) { junco::Log::standard("{}", msg); };
  auto threads = std::vector<std::thread>();
  for (const auto &msg : messages) {
    threads.push_back(std::thread(print_message, msg));
  }
  for (auto &thread : threads) {
    thread.join();
  }

  // Restore cout
  std::cout.rdbuf(old_rdbuf);

  // Search for all complete messages in the output message.
  // Messages are not guaranteed to be sent in the order above.
  auto output_str = output_strm.str();
  for (const auto &msg : messages) {
    auto message_is_valid = output_str.find(msg, 0) != std::string::npos;
    EXPECT_TRUE(message_is_valid);
  }
}

/**
 * Sends a log message through each channel to demonstrate their appearance.
 */
#ifdef DO_LOG_DEMO
TEST(LogTesting, DefaultLevelDemo) {
  junco::Log::trace("This is a trace message!");
  junco::Log::standard("This is a standard message!");
  junco::Log::warning("This is a warning!");
  junco::Log::error("This is an error!");
  junco::Log::fatal("This was a fatal error!");
}
#endif