#pragma once

#include "prsi/mgr/event/i_event.hpp"
#include <variant>

namespace prsi::mgr {

struct Leave_Room_Evin : public I_Evin {};

using Room_Evin = std::variant<Leave_Room_Evin>;

} // namespace prsi::mgr
