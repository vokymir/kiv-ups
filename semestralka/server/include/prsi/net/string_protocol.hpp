#pragma once

#include "prsi/mgr/event/in_event.hpp"
#include "prsi/mgr/event/out_event.hpp"
#include "prsi/net/protocol_handler.hpp"
#include <string>

namespace prsi::net {

// Protocol for PRSI using string.
struct String_Protocol_Tag {
  using Net_Msg_T = std::string;
  using In_Ev_T = mgr::In_Event;
  using Out_Ev_T = mgr::Out_Event;
  using State_T = int; // TODO: placeholder
};

} // namespace prsi::net
