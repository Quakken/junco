/**
 * @file junco/exception.hpp
 *
 * Defines junco's base exception class.
 */

#pragma once

#include <exception>

namespace junco {
class Exception : public std::exception {
public:
  Exception(const char *what) noexcept : msg(what) {};
  ~Exception() override = default;

  const char *what() const noexcept override { return msg; }

private:
  const char *msg;
};
} // namespace junco