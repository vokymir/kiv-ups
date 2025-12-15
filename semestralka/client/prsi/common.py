from prsi.config import ST_UNNAMED

class Card:
    def __init__(self, suit: str = "N", rank: str = "N") -> None:
        self.suit: str = suit
        self.rank: str = rank

    def valid(self) -> bool:
        valid_suit: bool = False
        valid_rank: bool = False
        match self.suit:
            case 'L':
                valid_suit = True
            case 'Z':
                valid_suit = True
            case 'K':
                valid_suit = True
            case 'S':
                valid_suit = True
            case _:
                valid_suit = False
        match self.rank:
            case '7':
                valid_rank = True
            case '8':
                valid_rank = True
            case '9':
                valid_rank = True
            case '0':
                valid_rank = True
            case 'J':
                valid_rank = True
            case 'Q':
                valid_rank = True
            case 'K':
                valid_rank = True
            case 'A':
                valid_rank = True
            case _:
                valid_rank = False

        return (valid_suit and valid_rank)

class Player:
    def __init__(self, nick: str, state: str = ST_UNNAMED) -> None:
        self.nick: str = nick
        self.state: str = state
        # for player I
        self.hand: list[Card] = []
        # for other players
        self.n_cards: int = 0

class Room:
    def __init__(self, id: int, state: str) -> None:
        self.id: int = id
        self.state: str = state
        self.players: list[Player] = []
        # is it our players turn?
        self.turn: bool = False

# to get rid of basedpyright errors
# actual client inherits from this
class Client_Dummy:
    def run(self) -> None:
        pass
    def connect(self, _ip: str, _port: int, _username: str) -> None:
        pass
    def rooms(self) -> None:
        pass
    def known_rooms(self) -> list[Room]:
        return []
    def join(self, _room_id: int) -> None:
        pass
    def disconnect(self) -> None:
        pass

    def draw_card(self) -> None:
        pass
    def leave_room(self) -> None:
        pass
    def play_card(self, _card_name: str) -> None:
        pass


