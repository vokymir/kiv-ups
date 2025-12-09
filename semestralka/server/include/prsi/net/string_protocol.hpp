#pragma once

#include "prsi/mgr/event/in_event.hpp"
#include "prsi/mgr/event/out_event.hpp"
#include "prsi/net/i_protocol.hpp"
#include <string>

namespace prsi::net {

// Only protocol in this project - uses string, no raw bytes.
class String_Protocol
    : public I_Protocol<std::string, mgr::In_Event, mgr::Out_Event, int> {

  std::string serialize(const mgr::Out_Event &ev) override;
  mgr::In_Event parse(const std::string &msg) override;
  bool validate(const int &state, const mgr::In_Event &ev) override;
  ~String_Protocol() override;
};

} // namespace prsi::net
