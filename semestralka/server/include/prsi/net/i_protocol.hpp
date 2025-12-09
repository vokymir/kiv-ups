#pragma once

namespace prsi::net {

// interface class for all protocols, to be easily interchangeable
template <typename Net_Msg_T, typename In_Ev_T, typename Out_Ev_T>
class I_Protocol {
  virtual Net_Msg_T serialize(Out_Ev_T &ev) = 0;
  virtual In_Ev_T parse(Net_Msg_T &msg) = 0;
  virtual ~I_Protocol() = default;
};

} // namespace prsi::net
