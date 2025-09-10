#include "junco/time.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <ratio>

using namespace std::chrono_literals;

// Comment out to ignore tests that use std::this_thread::sleep_for
// #define SLEEP_TESTS
// Comment out to ignore tests that print time info (GetLocalTime, GetLocalDate)
// #define ENABLE_COUT

#ifdef SLEEP_TESTS
#include <thread>
#endif
#ifdef ENABLE_COUT
#include <iostream>
#endif

class TestingClock {
public:
  using time_point = std::chrono::high_resolution_clock::time_point;
  static time_point now() noexcept { return time_point(time); }
  static void set_time(const std::chrono::milliseconds new_time) noexcept {
    time = new_time;
  }

private:
  inline static std::chrono::milliseconds time{};
};

constexpr bool close_enough(double a, double b) noexcept {
  constexpr double epsilon = 0.05;
  double distance = a - b;
  if (distance < 0)
    distance *= -1;
  return distance <= epsilon;
}

/**
 * Tests a clock's ability to bind to a different clock implementation depending
 * on user's needs.
 */
TEST(ClockTesting, Templating) {
  // This should compile
  auto c1 = junco::Clock{};
  // So should this
  auto c2 = junco::Clock<TestingClock>{};
  // This should not!
  // auto c3 = junco::Clock<std::string>{};
}

/**
 * Makes sure junco's clocks can bind to multiple implementations and still
 * recieve accurate time results.
 */
TEST(ClockTesting, NonDefaultClocks) {
  auto c1 = junco::Clock<std::chrono::system_clock>{};
  auto c2 = junco::Clock<std::chrono::high_resolution_clock>{};
  EXPECT_TRUE(close_enough(c1.get_time(), c2.get_time()));
#ifdef SLEEP_TESTS
  for (auto sleep_time = 0ms; sleep_time < 25ms; ++sleep_time) {
    std::this_thread::sleep_for(sleep_time);
    EXPECT_TRUE(close_enough(c1.get_time(), c2.get_time()));
  }
#endif
}

/**
 * Tests Clocks' ability to retrieve the correct time (in seconds and
 * milliseconds)
 */
TEST(ClockTesting, GetTime) {
  auto clock = junco::Clock<TestingClock>();
  auto times_sec = {1s, 2s, 3s, 4s, 5s};
  for (const auto &actual_time : times_sec) {
    TestingClock::set_time(actual_time);
    auto measured_time = clock.get_time();
    EXPECT_EQ(measured_time, actual_time.count());
  }
  auto times_ms = {1ms, 2ms, 3ms, 4ms, 5ms};
  for (const auto &actual_time : times_ms) {
    TestingClock::set_time(actual_time);
    auto measured_time = clock.get_time();
    EXPECT_EQ(measured_time,
              (double)actual_time.count() / (double)std::milli::den);
  }
  TestingClock::set_time(0s);
}

/**
 * Ensures clocks report accurate local time.
 */
#ifdef ENABLE_COUT
TEST(ClockTesting, GetLocalTime) {
  // Local time is hard to test, because it can't really be mocked up.
  // Right now, all clocks just rely on std::chrono::system_clock for local time
  // info. If this changes in the future, be sure to update this test.
  auto clock = junco::Clock{};
  auto local_tm = clock.get_local_time();
  std::cout << local_tm << std::endl;
}
#endif

/**
 * Ensures clocks report accurate date information.
 */
#ifdef ENABLE_COUT
TEST(ClockTesting, GetLocalDate) {
  // Same issue as GetLocalTime.
  auto clock = junco::Clock{};
  auto local_date = clock.get_local_date();
  std::cout << local_date << std::endl;
}
#endif

/**
 * Makes sure that stopwatches are linked to the correct clock/clock
 * implementation
 */
TEST(StopwatchTesting, Templating) {
  auto c1 = junco::Clock{};
  auto sw1 = junco::Stopwatch(c1);
  using c1_impl_t = decltype(c1)::impl_t;
  using sw1_impl_t = decltype(sw1)::clock_t::impl_t;
  constexpr auto impl_1_is_same = std::is_same_v<c1_impl_t, sw1_impl_t>;
  EXPECT_TRUE(impl_1_is_same);

  auto c2 = junco::Clock<TestingClock>{};
  auto sw2 = junco::Stopwatch(c2);
  using c2_impl_t = decltype(c2)::impl_t;
  using sw2_impl_t = decltype(sw2)::clock_t::impl_t;
  constexpr auto impl_2_is_same = std::is_same_v<c2_impl_t, sw2_impl_t>;
  EXPECT_TRUE(impl_2_is_same);
}

/**
 * Big test to make sure that stopwatches report accurate times.
 */
TEST(StopwatchTesting, GetTime) {
  auto clock = junco::Clock<TestingClock>{};
  auto sw1 = junco::Stopwatch(clock);
  auto sw2 = junco::Stopwatch(clock);

  // Make sure stopwatches report same times
  sw1.start();
  sw2.start();
  TestingClock::set_time(10s);
  EXPECT_EQ(sw1.stop(), sw2.stop());
  TestingClock::set_time(4s);
  sw1.start();
  sw2.start();
  TestingClock::set_time(10s);
  EXPECT_EQ(sw1.stop(), sw2.stop());

  // Make sure that a stopped stopwatch doesn't report any time
  EXPECT_EQ(sw1.get_time(), 0);
  EXPECT_EQ(sw1.stop(), 0);

  // Make sure stopwatches report accurate times
  TestingClock::set_time(0s);
  sw1.start();
  TestingClock::set_time(10s);
  EXPECT_EQ(sw1.stop(), 10);
  TestingClock::set_time(4s);
  sw1.start();
  TestingClock::set_time(10s);
  EXPECT_EQ(sw1.stop(), 6);

  TestingClock::set_time(0s);
}