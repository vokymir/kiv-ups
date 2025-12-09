#pragma once

#include <variant>

namespace prsi::mgr {

struct Leave_Room_Evin {};

using Room_Evin = std::variant<Leave_Room_Evin>;

} // namespace prsi::mgr
