#pragma once

#include "player.hpp"
#include <bits/types/wint_t.h>
#include <cctype>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <vector>

namespace prsi {

class Protocol {
public:
  static std::string PING() { return build_message("PING"); }

  // TODO: this is only proof of concept
  // Can do dynamic thingies
  static std::string CARDS(std::shared_ptr<Player> p) {
    std::string body = "CARDS " + std::to_string(p->fd());

    for (auto i : {10, 20}) {
      body += " " + std::to_string(i);
    }

    return build_message(body);
  }

  // return a message split by whitespaces into vector,
  // if no complete message, return empty vector
  // remove found messafge from mutable_string
  static std::vector<std::string> extract_message(std::string &mutable_string) {
    // find if any message exists
    auto delim_start = mutable_string.find(DELIM);
    if (delim_start == std::string::npos) {
      return {};
    }

    // extract the message
    std::string_view msg_view{mutable_string.c_str(),
                              delim_start + DELIM.size()};

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

  // validate any string without mutating
  static bool valid(const std::string &msg) {
    size_t i = 0;

    // skip whitespaces on the beginning
    while (i < msg.size() && std::isspace(msg[i])) {
      i++;
    }

    // check if have MAGIC on starting position
    return msg.compare(i, MAGIC.size(), MAGIC) == 0;
  }

private:
  static inline const std::string MAGIC = "PRSI";
  static inline const std::string DELIM = "|";

  // format any message into valid protocol message
  static std::string build_message(const std::string &body) {
    // better having more white spaces than less
    return " " + MAGIC + " " + body + " " + DELIM + "\n";
  }
};

} // namespace prsi
