#pragma once

#include "prsi/util/Config.hpp"
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
  Card(Barva b, Hodnota h) : barva(b), hodnota(h) {}
};

enum Card_Position {
  CARD_IN_DECK,
  CARD_IN_HAND,
  CARD_PLAYED,
};

struct Playing_Card {
  Card card_;
  Card_Position position_;
  union {
    util::player_id pid_;
    int deck_pos_;
  } where_;
};

} // namespace prsi::game
