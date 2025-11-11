#pragma once

namespace prsi::game {

enum class Suit : char {
  Hearts = 'H',
  Diamonds = 'D',
  Clubs = 'C',
  Spades = 'S'
};
enum class Rank : int {
  Seven = 7,
  Eight = 8,
  Nine = 9,
  Ten = 10,
  Under = 11,
  Over = 12,
  King = 13,
  Ace = 14
};

struct Card {
private:
  Suit suit_;
  Rank rank_;

public:
  Card();
  ~Card();
  // getters
  Suit Suit();
  Rank Rank();
};

} // namespace prsi::game
