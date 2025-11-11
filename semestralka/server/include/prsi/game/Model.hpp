#pragma once

#include "prsi/interfaces/ILoggable.hpp"
#include <memory>
#include <string>
#include <vector>

namespace prsi::game {

enum struct Barva : char {
  Srdce = 's',
  Listy = 'l',
  Kule = 'k',
  Zaludy = 'z',
};

enum struct Hodnota : char {
  Sedm = '7',
  Osm = '8',
  Devet = '9',
  Deset = '0',
  Spodek = 's',
  Menic = 'm',
  Kral = 'k',
  Eso = 'e',
};

struct Card {
  Barva barva;
  Hodnota hodnota;
  static std::vector<Card> create_deck();
};

struct Player {
  std::string nickname;
  int session_fd;
};

struct Room {
  std::vector<Player *> players;
  int n_players;
  std::vector<Card> deck;
  std::vector<Card> pile;
};

struct Lobby {
  std::vector<Player *> players;
  std::vector<std::unique_ptr<Room>> rooms;
};

struct Model : public interfaces::ILoggable {
  Lobby lobby;
  std::vector<std::unique_ptr<Player>> all_players;
};

} // namespace prsi::game
