
#include "prsi/net/session.hpp"
namespace prsi::net {

Session::Session(const util::Config &cfg) {}

Session::~Session() {
  // TODO: should send message of type: user leave, to the room in which it is
}

} // namespace prsi::net
