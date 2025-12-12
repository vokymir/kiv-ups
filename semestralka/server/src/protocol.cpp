
#include "protocol.hpp"

namespace prsi {

std::vector<std::string>
Protocol::extract_message(std::string &mutable_string) {
  // find if any message exists
  auto delim_start = mutable_string.find(DELIM);
  if (delim_start == std::string::npos) {
    return {};
  }

  // extract the message
  std::string_view msg_view{mutable_string.c_str(), delim_start + DELIM.size()};

  // split into parts, by whitespaces
  std::vector<std::string> result;
  size_t i = 0;
  while (i < msg_view.size()) {
    // skip whitespaces
    while (i < msg_view.size() &&
           std::isspace(static_cast<unsigned char>(msg_view[i]))) {
      i++;
    }
    // the whole msg only whitespaces - shouldn't happen, we have delim
    if (i >= msg_view.size()) {
      break;
    }

    size_t word_start = i;
    while (i < msg_view.size() &&
           !std::isspace(static_cast<unsigned char>(msg_view[i]))) {
      i++;
    }

    // extract word
    auto word = msg_view.substr(word_start, i - word_start);

    // dont include magic and delim in result
    if (MAGIC.compare(word) != 0 && DELIM.compare(word) != 0) {
      result.emplace_back(word);
    }
  }

  // erase from input string
  mutable_string.erase(0, delim_start + DELIM.size());

  return result;
}

bool Protocol::could_validate(const std::string &msg) {
  if (msg.empty()) {
    return false;
  }

  // skip whitespaces on the beginning
  size_t i = 0;
  while (i < msg.size() && std::isspace(msg[i])) {
    i++;
  }

  // could be compared?
  return msg.size() > i + MAGIC.size();
}

bool Protocol::valid(const std::string &msg) {
  size_t i = 0;

  // skip whitespaces on the beginning
  while (i < msg.size() && std::isspace(msg[i])) {
    i++;
  }

  // check if have MAGIC on starting position
  return msg.compare(i, MAGIC.size(), MAGIC) == 0;
}

} // namespace prsi
