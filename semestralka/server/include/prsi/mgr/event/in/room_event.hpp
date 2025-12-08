#pragma once

#include <variant>

namespace prsi::mgr {

struct Leave_Room_Event {};

using Room_Event = std::variant<Leave_Room_Event>;

} // namespace prsi::mgr
