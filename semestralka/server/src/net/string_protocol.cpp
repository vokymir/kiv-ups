#include "prsi/net/string_protocol.hpp"
#include "prsi/mgr/event/in_event.hpp"
#include "prsi/mgr/event/out_event.hpp"
#include <string>

namespace prsi::net {

// TODO:
std::string String_Protocol::serialize(const mgr::Out_Event &ev) {
  return "TODO:";
}

// TODO:
mgr::In_Event String_Protocol::parse(const std::string &msg) {
  return mgr::In_Event{};
}

// TODO:
bool String_Protocol::validate(const int &state, const mgr::In_Event &ev) {
  return true;
}

} // namespace prsi::net
