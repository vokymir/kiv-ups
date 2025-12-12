#include <array>
#include <cstddef>
#include <string>

namespace prsi {
// is compiled and no overhead added

// any magic to start message, any delim to end message
constexpr char MAGIC[] = "PRSI ";
constexpr char DELIM[] = "|";

// create mesage of any length, with any magic and any delim
// N = msg.size()
// M = magic.size()
// L = delim.size()
template <std::size_t N, std::size_t M, std::size_t L>
constexpr auto make_any_message(const char (&msg)[N], const char (&magic)[M],
                                const char (&delim)[L]) {
  std::array<char, M + N + L - 2> result{}; // -2 because of null terminators
  std::size_t i = 0;

  // copy magic
  for (std::size_t j = 0; j < M - 1; ++j)
    result[i++] = magic[j];
  // copy original message
  for (std::size_t j = 0; j < N - 1; ++j)
    result[i++] = msg[j];
  // copy delimiter
  for (std::size_t j = 0; j < L - 1; ++j)
    result[i++] = delim[j];

  return result;
}

// create message for my protocol
// N = msg.size()
template <std::size_t N> constexpr auto make_message(const char (&msg)[N]) {
  return make_any_message(msg, MAGIC, DELIM);
}

// wrapper for any message
template <typename T> struct Message {
  const T value;

  // Implicit conversion to std::string
  // -> in compile time resolved
  operator std::string() const {
    return std::string(value.data(), value.size());
  }
};

// store all messages the server needs to send
struct Protocol {
  static constexpr Message PING{make_message("PING")};
};

} // namespace prsi
