#pragma once

#include <string>
namespace prsi {

struct Card {
  char rank_ = 'N'; // invalid
  char suit_ = 'N';

  Card() {}
  Card(char suit, char rank) : rank_(rank), suit_(suit) {}

  bool is_valid() {
    bool valid_rank = false;
    switch (rank_) {
    case '7':
    case '8':
    case '9':
    case '0': // 10
    case 'J':
    case 'Q': // menic
    case 'K':
    case 'A':
      valid_rank = true;
    }

    bool valid_suit = false;
    switch (suit_) {
    case 'Z': // zaludy
    case 'L': // listy
    case 'K': // kule
    case 'S': // srdce
      valid_suit = true;
    }

    return valid_rank && valid_suit;
  }

  std::string to_string() const { return std::string{suit_} + rank_; }
};

} // namespace prsi
