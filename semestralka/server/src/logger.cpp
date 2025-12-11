
#include "logger.hpp"
#include <chrono>
#include <format>
#include <iostream>
namespace prsi {

std::mutex Logger::log_mutex_;

std::string get_formatted_time() {
  auto now = std::chrono::system_clock::now();

  std::string formatted_time = std::format("{:%F %T}", now);

  return formatted_time;
}

void Logger::log(std::string_view severity, std::string_view msg) {
  std::lock_guard<std::mutex> lock(log_mutex_);

  std::string formatted_log =
      std::format("[{} | {}] {}", severity, get_formatted_time(), msg);

  std::cerr << formatted_log << std::endl;
}

} // namespace prsi
