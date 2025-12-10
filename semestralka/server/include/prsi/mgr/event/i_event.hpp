#pragma once

namespace prsi::mgr {

// what should be passed in all Events
struct I_Event {
  int fd; // session fd
};

// what should be passed in all In_Events
struct I_Evin : public I_Event {};

// what should be passed in all Out_Events
struct I_Evout : public I_Event {};

} // namespace prsi::mgr
