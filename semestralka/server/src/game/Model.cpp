#include "prsi/game/Model.hpp"
#include <vector>

namespace prsi::game {

std::vector<Card> Card::create_deck() {
  std::vector<Card> cards;

  for (Barva b : {Barva::Srdce, Barva::Listy, Barva::Kule, Barva::Zaludy}) {
    for (Hodnota h :
         {Hodnota::Sedm, Hodnota::Osm, Hodnota::Devet, Hodnota::Deset,
          Hodnota::Spodek, Hodnota::Menic, Hodnota::Kral, Hodnota::Eso}) {
      cards.emplace_back(b, h);
    }
  }

  return cards;
}

} // namespace prsi::game
