#pragma once

#include "server/Message.hpp"
#include <string>

namespace prsi::server {

class Protocol {
public:
  static const Client_Message parse(const std::string &raw);
  static const std::string serialize(const Server_Message msg);
};

} // namespace prsi::server
