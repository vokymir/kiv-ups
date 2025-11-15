#pragma once

namespace prsi::game {

enum class Rank {
  SEDM = '7',
  OSM = '8',
  DEVET = '9',
  DESET = '0',
  SPODEK = 'S',
  MENIC = 'M',
  KRAL = 'K',
  ESO = 'E',
};

enum class Suit {
  SRDCE = 'S',
  ZALUDY = 'Z',
  LISTY = 'L',
  KULE = 'K',
};

class Card {
private:
  Rank rank_;
  Suit suit_;

public:
  const Rank &rank() { return rank_; }
  const Suit &suit() { return suit_; }
};

} // namespace prsi::game
