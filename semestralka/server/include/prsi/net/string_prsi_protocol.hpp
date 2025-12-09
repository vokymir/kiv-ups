#pragma once

#include "i_protocol.hpp"
#include "prsi/mgr/event/in_event.hpp"
#include "prsi/mgr/event/out_event.hpp"
#include <string>

namespace prsi::net {

// Only protocol in this project - uses string, no raw bytes.
class String_Prsi_Protocol
    : public I_Protocol<std::string, mgr::In_Event, mgr::Out_Event> {

  std::string serialize(mgr::In_Event &ev) override;
  mgr::Out_Event parse(std::string &msg) override;
  ~String_Prsi_Protocol() override;
};

} // namespace prsi::net
