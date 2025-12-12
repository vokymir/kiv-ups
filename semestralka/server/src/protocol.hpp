#include "player.hpp"
#include <memory>
#include <string>

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

private:
  // format any message into valid protocol message
  static std::string build_message(const std::string &body) {
    static const std::string MAGIC = "PRSI";
    static const std::string DELIM = "|";
    return MAGIC + " " + body + " " + DELIM;
  }
};

} // namespace prsi
