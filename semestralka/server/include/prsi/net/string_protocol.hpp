#pragma once

#include "i_protocol.hpp"

namespace prsi::net {

// Only protocol in this project - uses string, no raw bytes.
class String_Protocol : public I_Protocol {};

} // namespace prsi::net
