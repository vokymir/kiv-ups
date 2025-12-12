#pragma once

#include "player.hpp"
#include <cstddef>
#include <fmt/format.h>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

namespace prsi {

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

  // protect shared resource - the place which to log into
  static std::mutex log_mutex_;

public:
  // public interface
  static inline constexpr Generic_Log<"INFO"> info{};
  static inline constexpr Generic_Log<"WARN"> warn{};
  static inline constexpr Generic_Log<"EROR"> error{};

  // easily show more info about something

  // more info about player
  static inline const std::string more(const std::string &msg,
                                       std::shared_ptr<Player> p) {
    std::string response = "Player ";
    if (!p->nick().empty()) {
      response += p->nick() + " ";
    }
    response += "fd=" + std::to_string(p->fd()) + ": ";

    response += msg;

    return response;
  }
};

} // namespace prsi
