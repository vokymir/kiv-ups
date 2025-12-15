
class Room:
    def __init__(self, id: int, state: str) -> None:
        self.id: int = id
        self.state: str = state

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
