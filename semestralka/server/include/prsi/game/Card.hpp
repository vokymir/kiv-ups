#pragma once

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
  Barva barva_;
  Hodnota hodnota_;
};

} // namespace prsi::game
