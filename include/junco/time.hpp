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
 *
 * @tparam T Type of clock to use for measurements (default
 * std::chrono::high_resolution_clock)
 */
template <typename T = std::chrono::high_resolution_clock> class Clock final {
public:
  using impl_t = T;

  Clock() noexcept : start_time(T::now()) {}

  /**
   * Returns the time, in seconds, since the clock was created.
   */
  double get_time() const noexcept {
    auto duration = T::now() - start_time;
    return std::chrono::duration_cast<std::chrono::duration<double>>(duration)
        .count();
  }

  Time get_local_time() const noexcept {
    auto local_time = get_local_raw();
    auto days = std::chrono::floor<std::chrono::days>(local_time);
    auto hms = std::chrono::hh_mm_ss(local_time - days);
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(hms.subseconds());
    return Time{.hours = hms.hours(),
                .minutes = hms.minutes(),
                .seconds = hms.seconds(),
                .milliseconds = millis};
  }
  Date get_local_date() const noexcept {
    auto local_time = get_local_raw();
    auto days = std::chrono::floor<std::chrono::days>(local_time);
    auto ymd = std::chrono::year_month_day(days);
    return Date{
        .month = ymd.month(),
        .day = ymd.day(),
        .year = ymd.year(),
        .weekday = std::chrono::weekday(days),
    };
  }

private:
  auto get_local_raw() const noexcept {
    auto utc_now = std::chrono::system_clock::now();
    auto local_now = std::chrono::current_zone()->to_local(utc_now);
    return local_now;
  }

  T::time_point start_time;
};

/**
 * Provides utilities for measuring time elapsed between two points.
 */
template <typename T = std::chrono::high_resolution_clock>
class Stopwatch final {
public:
  using clock_t = Clock<T>;

  Stopwatch(const clock_t &_clock) noexcept
      : clock(_clock), start_time(0), is_started(false) {}
  Stopwatch(const Stopwatch &other) noexcept
      : clock(other.clock), start_time(0), is_started(false) {}
  ~Stopwatch() = default;

  void operator=(const Stopwatch &) = delete;

  void start() noexcept {
    if (is_started)
      return;
    is_started = true;
    start_time = clock.get_time();
  }
  double get_time() noexcept {
    return (is_started ? clock.get_time() - start_time : 0);
  }
  /**
   * Stops the stopwatch, returning the time before it was stopped.
   * Subsequent calls to get_time() will return 0.
   */
  double stop() noexcept {
    if (!is_started)
      return 0;
    auto time = get_time();
    is_started = false;
    return time;
  }

  bool started() const noexcept { return is_started; }

private:
  const clock_t &clock;
  double start_time;
  bool is_started;
};
} // namespace junco