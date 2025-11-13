#pragma once

#include "prsi/util/Config.hpp"
#include <vector>

namespace prsi::game {

struct Lobby {
  std::vector<util::player_id> player_ids_;
};

} // namespace prsi::game
