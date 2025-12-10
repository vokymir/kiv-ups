#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Ping_Evout : public I_Evout {};

using Control_Evout = std::variant<Ping_Evout>;

} // namespace prsi::mgr
