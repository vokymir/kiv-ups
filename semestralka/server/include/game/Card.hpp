#pragma once

#include <array>
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

inline constexpr std::array<Rank, 8> ALL_RANKS = {
    Rank::SEDM,   Rank::OSM,   Rank::DEVET, Rank::DESET,
    Rank::SPODEK, Rank::MENIC, Rank::KRAL,  Rank::ESO};

enum class Suit {
  SRDCE = 'S',
  ZALUDY = 'Z',
  LISTY = 'L',
  KULE = 'K',
};

inline constexpr std::array<Suit, 4> ALL_SUITS = {Suit::SRDCE, Suit::ZALUDY,
                                                  Suit::LISTY, Suit::KULE};

class Card {
private:
  Rank rank_;
  Suit suit_;

public:
  const Rank &rank() const { return rank_; }
  const Suit &suit() const { return suit_; }

  Card(Rank rank, Suit suit) : rank_(rank), suit_(suit) {}
};

} // namespace prsi::game
