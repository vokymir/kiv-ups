#pragma once

#include "prsi/mgr/event/in_event.hpp"
#include "prsi/mgr/event/out_event.hpp"
#include "prsi/mgr/lobby.hpp"
#include "prsi/mgr/room.hpp"
#include <deque>
#include <memory>
#include <vector>

namespace prsi::mgr {

// Own lobby, rooms, pass events to them or back to server
class Game_Manager {
private:
  std::unique_ptr<Lobby> lobby_;
  std::vector<std::unique_ptr<Room>> rooms_;
  std::deque<Out_Event> outgoing_events_;

public:
  // process evs, remove from deque, fill out_evs deque
  void process(std::deque<In_Event> &evs);

  // make outgoing_events_ accessible to read/write/remove
  std::deque<Out_Event> &out();
};

} // namespace prsi::mgr
