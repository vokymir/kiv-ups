#pragma once

namespace prsi {

struct Card {
  char rank_;
  char suit_;

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
};

} // namespace prsi
