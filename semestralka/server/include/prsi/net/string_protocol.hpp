#pragma once

#include "prsi/mgr/event/in_event.hpp"
#include "prsi/mgr/event/out_event.hpp"
#include "prsi/net/i_protocol.hpp"
#include <string>

namespace prsi::net {

// all possible (allowed) state the session could be in
enum Session_State {
  Connected,
  In_Lobby,
  In_Room,
  Waiting_On_Turn,
  Playing,
  Game_Finished,
};

// Only protocol in this project - uses string, no raw bytes.
class String_Protocol : public I_Protocol<std::string, mgr::In_Event,
                                          mgr::Out_Event, Session_State> {
public:
  std::string serialize(const mgr::Out_Event &ev) override;
  mgr::In_Event parse(const std::string &msg) override;
  bool validate(const Session_State &state, const mgr::In_Event &ev) override;

  String_Protocol();
  ~String_Protocol();
};

} // namespace prsi::net
