#pragma once

#include <fmt/format.h>
#include <string_view>

namespace prsi::util {

// Helper so can use strings in later template
template <size_t N> struct Log_Severity {
  char value[N]{};

  consteval Log_Severity(const char (&str)[N]) {
    for (size_t i = 0; i < N; ++i)
      value[i] = str[i];
  }

  constexpr operator std::string_view() const {
    return {value, N - 1};
  } // drop '\0'
};

class Logger {
private:
  // delete con(de)structor
  Logger();
  ~Logger();
  // Delete copy/move
  Logger(const Logger &) = delete;
  Logger(Logger &&) = delete;
  Logger &operator=(const Logger &) = delete;
  Logger &operator=(Logger &&) = delete;

  // write log to file
  static void log(std::string_view severity, std::string_view msg);

  // template for multiple log severities, each severity defined by string
  template <Log_Severity Severity> struct Generic_Log {
    template <typename... Args>
    void operator()(fmt::format_string<Args...> fmt_str, Args &&...args) const {
      Logger::log(Severity, fmt::format(fmt_str, std::forward<Args>(args)...));
    }
  };

public:
  // public interface
  static inline constexpr Generic_Log<"INFO"> info{};
  static inline constexpr Generic_Log<"WARN"> warn{};
  static inline constexpr Generic_Log<"EROR"> error{};
};

} // namespace prsi::util
