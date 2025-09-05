/**
 * @file junco/time.hpp
 *
 * Clocks provide a uniform interface for accessing date/time related
 * information.
 *
 * They are used by the engine to calculate delta time and to create
 * stopwatches, which are used for profiling.
 */

#pragma once
#include <chrono> // std::chrono

namespace junco {
class Stopwatch;

/**
 * Stores information about local time, using std::chrono units.
 */
struct Time {
  std::chrono::hours hours;
  std::chrono::minutes minutes;
  std::chrono::seconds seconds;
  std::chrono::milliseconds milliseconds;
  friend std::ostream &operator<<(std::ostream &lhs, const Time &rhs) noexcept {
    return lhs << rhs.hours.count() << ':' << rhs.minutes.count() << ':'
               << rhs.seconds.count() << '.' << rhs.milliseconds.count();
  }
};

/**
 * Stores information about a date, using std::chrono units.
 */
struct Date {
  std::chrono::month month;
  std::chrono::day day;
  std::chrono::year year;
  std::chrono::weekday weekday;
  friend std::ostream &operator<<(std::ostream &lhs, const Date &rhs) noexcept {
    // TODO: Allow for formatting other than www__mmm_dd_yyyy
    return lhs << rhs.weekday << ", " << rhs.month << " " << rhs.day << ", "
               << rhs.year;
  }
};

/**
 * Object that provides a uniform interface for measuring elapsed time and
 * retrieving local time/date information.
 */
class Clock final {
public:
  Clock() noexcept;

  /**
   * Returns the time, in seconds, since the clock was created.
   */
  double get_time() const noexcept;

  Time get_local_time() const noexcept;
  Date get_local_date() const noexcept;

  /**
   * Creates a new stopwatch which uses this clock's time info.
   */
  Stopwatch make_stopwatch() const noexcept;

private:
  using chrono_clock = std::chrono::high_resolution_clock;

  auto get_local_raw() const noexcept {
    auto utc_now = std::chrono::system_clock::now();
    auto local_now = std::chrono::current_zone()->to_local(utc_now);
    return local_now;
  }

  chrono_clock::time_point start_time;
};

/**
 * Provides utilities for measuring time elapsed between two points.
 */
class Stopwatch final {
public:
  Stopwatch(const Clock &) noexcept;
  Stopwatch(const Stopwatch &other) noexcept;
  ~Stopwatch() = default;

  void operator=(const Stopwatch &) = delete;

  void start() noexcept;
  double get_time() noexcept;
  /**
   * Stops the stopwatch, returning the time before it was stopped.
   * Subsequent calls to get_time() will return 0.
   */
  double stop() noexcept;

  bool started() const noexcept;

private:
  const Clock &clock;
  double start_time;
  bool is_started;
};
} // namespace junco