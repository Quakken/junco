#include "junco/time.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <thread> // std::this_thread::sleep_for

TEST(ClockTests, Time) {
  auto clock = junco::Clock{};
  ASSERT_GE(clock.get_time(), 0);
}

TEST(ClockTests, LocalTime) {
  auto clock = junco::Clock{};
  std::cout << clock.get_local_date() << std::endl;
  std::cout << clock.get_local_time() << std::endl;
}

TEST(StopwatchTests, InvalidStart) {
  auto clock = junco::Clock{};
  auto sw = junco::Stopwatch(clock);
  ASSERT_EQ(sw.stop(), 0);
  ASSERT_FALSE(sw.started());
}

// TODO: Make this not use std::this_thread::sleep_for
// Introduce mock std::chrono clocks to pass to clock's template
TEST(StopwatchTests, RecordTime) {
  using namespace std::chrono_literals;
  auto clock = junco::Clock{};
  auto sw = junco::Stopwatch(clock);
  sw.start();
  std::this_thread::sleep_for(1s);
  ASSERT_GE(sw.stop(), 1);
  ASSERT_EQ(sw.stop(), 0);
  ASSERT_EQ(sw.get_time(), 0);
}