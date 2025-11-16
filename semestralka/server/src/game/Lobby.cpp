#include "game/Lobby.hpp"
#include "game/Room.hpp"
#include "util/Config.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>
namespace prsi::game {

const std::vector<Room> &Lobby::get_rooms() const { return rooms_; }

const Room *Lobby::get_room(int room_id) const {
  auto it =
      std::find_if(rooms_.begin(), rooms_.end(), [room_id](const Room &room) {
        return room.id() == room_id;
      });

  return (it != rooms_.end()) ? &(*it) : nullptr;
}

int Lobby::get_next_room_id() const {
  if (rooms_.empty()) {
    return 0;
  }

  // get all ids in use
  std::vector<int> ids;
  ids.reserve(rooms_.size());
  std::transform(rooms_.begin(), rooms_.end(), std::back_inserter(ids),
                 [](const Room &room) { return room.id(); });
  std::sort(ids.begin(), ids.end());

  // find first non-used id, starting at 0 and going up
  for (int i = 0; i < static_cast<int>(ids.size()); i++) {
    if (ids[static_cast<size_t>(i)] != i) {
      return i;
    }
  }

  // return max
  return static_cast<int>(ids.size());
}

int Lobby::add_room() {
  // already max rooms
  if (rooms_.size() >=
      static_cast<size_t>(util::Config::instance().max_rooms())) {
    return -1;
  }

  int room_id = get_next_room_id();
  rooms_.emplace_back(Room{room_id});
  return room_id;
}

void Lobby::remove_room(int room_id) {
  auto it =
      std::find_if(rooms_.begin(), rooms_.end(), [room_id](const Room &room) {
        return room.id() == room_id;
      });

  if (it != rooms_.end()) {
    rooms_.erase(it);
  }
}

} // namespace prsi::game
