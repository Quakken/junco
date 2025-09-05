#include "junco/time.hpp"
#include <chrono>

namespace junco {
Clock::Clock() noexcept : start_time(chrono_clock::now()) {}

double Clock::get_time() const noexcept {
  auto duration = chrono_clock::now() - start_time;
  return std::chrono::duration_cast<std::chrono::duration<double>>(duration)
      .count();
}

Time Clock::get_local_time() const noexcept {
  auto local_time = get_local_raw();
  auto days = std::chrono::floor<std::chrono::days>(local_time);
  auto hms = std::chrono::hh_mm_ss(local_time - days);
  return Time{.hours = hms.hours(),
              .minutes = hms.minutes(),
              .seconds = hms.seconds(),
              .milliseconds =
                  std::chrono::duration_cast<std::chrono::milliseconds>(
                      hms.subseconds())};
}
Date Clock::get_local_date() const noexcept {
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

Stopwatch Clock::make_stopwatch() const noexcept { return Stopwatch(*this); }

Stopwatch::Stopwatch(const Clock &_clock) noexcept
    : clock(_clock), start_time(0), is_started(false) {}
Stopwatch::Stopwatch(const Stopwatch &other) noexcept
    : clock(other.clock), start_time(0), is_started(false) {}

void Stopwatch::start() noexcept {
  is_started = true;
  start_time = clock.get_time();
}
double Stopwatch::get_time() noexcept {
  return (is_started ? clock.get_time() - start_time : 0);
}
double Stopwatch::stop() noexcept {
  auto time = get_time();
  is_started = false;
  return time;
}
bool Stopwatch::started() const noexcept { return is_started; }

} // namespace junco