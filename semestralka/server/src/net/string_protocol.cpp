#include "prsi/net/string_protocol.hpp"
#include "prsi/net/protocol_handler.hpp"

namespace prsi::net {

using Tag = String_Protocol_Tag;
using Handler = Protocol_Handler<Tag>;

template <> Tag::Net_Msg_T Handler::serialize(const Tag::Out_Ev_T &ev) {
  return "Hello";
}

template <> Tag::In_Ev_T Handler::parse(const Tag::Net_Msg_T &msg) {}

} // namespace prsi::net
