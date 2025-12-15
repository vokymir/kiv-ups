
#include "logger.hpp"
#include <chrono>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
namespace prsi {

std::mutex Logger::log_mutex_;

std::string get_formatted_time() {
  auto now = std::chrono::system_clock::now();

  return fmt::format("{:%F %T}", now);
}

void Logger::log(std::string_view severity, std::string_view msg) {
  std::lock_guard<std::mutex> lock(log_mutex_);

  std::string formatted_log =
      fmt::format("[{} | {}] {}", severity, get_formatted_time(), msg);

  std::cerr << formatted_log << '\n';
}

} // namespace prsi
