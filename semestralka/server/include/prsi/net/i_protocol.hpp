#pragma once

namespace prsi::net {

// interface class for all protocols, to be easily interchangeable
// Net_Msg_T    = message type send via net - e.g. binary/text
// In_Ev_T      = event which is used on server and is specific to incoming
// Out_Ev_T     = event specific for outgoing events server -> client
// State_T      = client's state in a game for example
//              used for validating incoming event if server isn't state-less
template <typename Net_Msg_T, typename In_Ev_T, typename Out_Ev_T,
          typename State_T>
class I_Protocol {

  // translate game-server event to net message type
  virtual Net_Msg_T serialize(const Out_Ev_T &ev) = 0;

  // parse net message type to game-client event
  virtual In_Ev_T parse(const Net_Msg_T &msg) = 0;

  // validate the game-client event can be played with current state of client
  // return true if can, false if can not
  virtual bool validate(const State_T &state, const In_Ev_T &ev);

  virtual ~I_Protocol() = default;
};

template <typename Tag> class Protocol_Handler {
public:
  using Net_Msg_T = typename Tag::Net_Msg_T;
  using In_Ev_T = typename Tag::In_Ev_T;
  using Out_Ev_T = typename Tag::Out_Ev_T;
  using State_T = typename Tag::State_T;

  // translate game-server event to net message type
  static Net_Msg_T serialize(const Out_Ev_T &ev);

  // parse net message type to game-client event
  static In_Ev_T parse(const Net_Msg_T &msg);

  // validate the game-client event can be played with current state of client
  // return true if can, false if can not
  static bool validate(const State_T &state, const In_Ev_T &ev);
};

} // namespace prsi::net
