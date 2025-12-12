#pragma once

#include "player.hpp"
#include <bits/types/wint_t.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

namespace prsi {

class Protocol {
public:
  // WRITE
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

  // = ok messages
  static std::string OK_NAME() { return build_message("OK NAME"); }

  // READ

  // return a message split by whitespaces into vector,
  // if no complete message, return empty vector
  // remove found messafge from mutable_string
  static std::vector<std::string> extract_message(std::string &mutable_string);

  // could the message even be validated - is long enough?
  static bool could_validate(const std::string &msg);

  // validate any string without mutating
  // does string start with magic?
  static bool valid(const std::string &msg);

private:
  // WRITE
  static inline const std::string MAGIC = "PRSI";
  static inline const std::string DELIM = "|";

  // format any message into valid protocol message
  static std::string build_message(const std::string &body) {
    // better having more white spaces than less
    // because in this protocol white spaces are ignored
    return " " + MAGIC + " " + body + " " + DELIM + "\n";
  }
};

} // namespace prsi
