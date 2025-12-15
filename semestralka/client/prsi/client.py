from queue import Queue
from tkinter import messagebox
import queue
from typing import override
from prsi.net import QM_DISCONNECTED, QM_ERROR, QM_MESSAGE, Net, Queue_Message
from prsi.ui import Ui
from prsi.config import CMD_DRAW, CMD_JOIN, CMD_NAME, CMD_PONG, CMD_ROOM, CMD_ROOMS, FN_LOBBY, FN_LOGIN, FN_ROOM, ST_GAME, ST_LOBBY
from prsi.common import Card, Client_Dummy, Player, Room

class Client(Client_Dummy):
    """
    The controller, have net & ui and actually do all the stuff.
    """
    def __init__(self) -> None:
        # message queue: for getting messages out of net
        self.mq: Queue[Queue_Message] = queue.Queue()

        # stuff
        # = not unnamed
        self.player: Player | None = None
        # = lobby
        self.known_rooms_: list[Room] = []
        # = room
        self.room: Room | None = None
        # = game

        # Already Sent DRAW
        self.as_draw: bool = False

        # network part of client - talk via queue
        self.net: Net = Net(self.mq)

        # ui
        self.ui: Ui = Ui(self)

        _ = self.ui.after(100, self.process_incoming_messages)
        print("[Client] Initialization complete. Ready to run.")

    @override
    def run(self) -> None:
        """
        Start main loop.
        """
        print("--- Starting Tkinter main loop (GUI should now appear) ---")
        self.ui.update()
        self.ui.mainloop()
        print("--- Tkinter main loop exited ---")

    # get/set

    @override
    def known_rooms(self) -> list[Room]:
        """
        State=lobby
        """
        return self.known_rooms_

    def opponent(self) -> Player | None:
        if (self.player and self.room and self.room.players):
            for player in self.room.players:
                if (player.nick != self.player.nick):
                    return player
        return None

    @override
    def opponent_n_cards(self) -> int:
        """
        State=game
        """
        p: Player | None = self.opponent()
        if (isinstance(p, Player)):
            return p.n_cards
        return 0

    @override
    def get_top_card(self) -> Card:
        """
        State=game
        """
        if (self.room):
            return self.room.top_card
        return Card()

    # ui -> net

    @override
    def connect(self, ip: str, port: int, username: str) -> None:
        if not(self.net.connect(ip, str(port))):
            self.ui.show_temp_message("Cannot connect to the server")
            return

        self.net.send_command(CMD_NAME + " " + username)
        self.player = Player(username) # save username here - after dont have it
        self.ui.show_temp_message("Trying to connect.", 1000)

    @override
    def rooms(self) -> None:
        """
        Ask server for rooms
        AS=none, State=Lobby
        """
        if (self.player and self.player.state == ST_LOBBY):
            self.net.send_command(CMD_ROOMS)
            return
        self.ui.show_temp_message("Cannot show rooms when not in lobby")

    @override
    def join(self, room_id: int) -> None:
        """
        AS=none, State=Lobby
        """
        if (self.player and self.player.state == ST_LOBBY):
            self.net.send_command(CMD_JOIN + " " + str(room_id))
            return
        self.ui.show_temp_message("Cannot join room when not in lobby")

    @override
    def disconnect(self) -> None:
        """
        AS=none
        """
        self.net.disconnect()

        self.player = None
        self.room = None
        self.known_rooms_ = []

        self.ui.switch_frame(FN_LOGIN)

    @override
    def draw_card(self) -> None:
        """
        AS=true, State=game
        """
        if (self.player and self.player.state == ST_GAME and \
        self.room and self.room.turn and (not self.as_draw)):
            self.net.send_command(CMD_DRAW)
            return
        self.ui.show_temp_message("It's not your turn.")

    # net -> ui

    def process_incoming_messages(self) -> None:
        """
        Periodically check the queue for messages from the net.
        This method runs on the main thread = safe to update tkinter.
        """
        while (not self.mq.empty()):
            qm_type, content = self.mq.get()

            if (qm_type == QM_ERROR):
                _ = messagebox.showerror("Network Error", content if content else "Unknown Error")
                self.ui.switch_frame(FN_LOGIN)

            elif (qm_type == QM_DISCONNECTED):
                self.ui.switch_frame(FN_LOGIN)

            elif (qm_type == QM_MESSAGE):
                if (content):
                    self.handle_protocol(content)

        _ = self.ui.after(100, self.process_incoming_messages)

    def handle_protocol(self, msg: str) -> None:
        """
        Authoritative place to handle all incoming messages from net.
        """
        print(f"[PROTO] Received: {msg}")
        parts: list[str] = msg.split()
        if (not parts):
            return

        cmd: str = parts[0]

        match cmd:
            case "OK":
                if (len(parts) > 1):
                    match parts[1]:
                        case "NAME":
                            if (not self.player):
                                self.ui.show_temp_message("WEIRD BUG #1")
                                self.disconnect()
                            else:
                                self.player.state = ST_LOBBY
                            self.net.send_command(CMD_ROOMS)
                        case "JOIN_ROOM":
                            self.net.send_command(CMD_ROOM)
                        case _:
                            pass

                return
            case "ROOMS":
                self.parse_rooms_message(parts)
                self.ui.refresh_lobby()
                self.ui.switch_frame(FN_LOBBY)
            case "ROOM":
                self.parse_room_message(parts)
                # don't interrupt eg. running game
                if (self.player and self.player.state == ST_LOBBY):
                    self.ui.lobby_frame.refresh_room_list()
                    self.ui.switch_frame(FN_ROOM)
            case "GAME_START":
                # maybe nothing?
                pass
            case "HAND":
                self.parse_hand_message(parts)
                if (self.player):
                    self.ui.room_frame.update_hand(self.player.hand)
            case "TURN":
                # update current turn, top card
                pass
            case "PLAYED":
                # show what other player did
                pass
            case "DRAWED":
                # show that other player drew
                pass
            case "SKIP":
                # maybe show sho is skipped
                pass
            case "CARDS":
                # add these cards to hand
                pass
            case "WIN":
                # show you are winner
                pass
            case "LOSE":
                # show you are loser
                pass
            case "PING":
                self.net.send_command(CMD_PONG)
            case "STATE":
                # change UI accordingly
                pass
            case "SLEEP":
                # show who is sleeping
                pass
            case "DEAD":
                # show who is dead
                pass
            case "AWAKE":
                # show who is awake
                pass
            case _:
                self.ui.show_temp_message("Received unknown message from server.")

    def parse_rooms_message(self, msg: list[str]) -> None:
        try:
            count: int = int(msg[1])
            rooms: list[Room] = []

            beg_idx: int = 2 # skip cmd, count
            ris: int = 2 # room info size
            for i in range(beg_idx, beg_idx + count * ris, ris):
                rooms.append(
                    Room(
                        int(msg[i]),
                        msg[i+1]
                ))

            # replace with new rooms
            self.known_rooms_ = rooms

        except Exception as e:
            print(f"[PROTO] invalid rooms message received ({" ".join(msg)})\
            resulting in: {e}")
            self.ui.show_temp_message("Cannot display rooms.")

    def parse_room_message(self, msg: list[str]) -> None:
        try:
            id: int = int(msg[1])
            state: str = msg[2]
            room: Room = Room(id, state)

            n_players: int = int(msg[4])
            # add all players
            for i in range(n_players):
                # start on 5, every player is 3 thingies
                idx: int = 5 + i * 3
                name: str = msg[idx]
                p_state: str = msg[idx+1]
                n_cards: int = int(msg[idx+2])

                player: Player = Player(name, p_state)
                player.n_cards = n_cards

                room.players.append(player)

            # set global my room
            self.room = room

        except Exception as e:
            print(f"[PROTO] invalid room message received ({" ".join(msg)})\
            resulting in: {e}")
            self.ui.show_temp_message("Cannot display room.")


    def parse_hand_message(self, msg: list[str]) -> None:
        try:
            count: int = int(msg[1])
            hand: list[Card] = []

            for i in range(2, 2+count):
                cd: str = msg[i]
                suit: str = cd[0]
                rank: str = cd[1]

                card: Card = Card(suit, rank)
                hand.append(card)

            if (self.player):
                self.player.hand = hand

        except Exception as e:
            print(f"[PROTO] invalid hand message received ({" ".join(msg)})\
            resulting in: {e}")
            self.ui.show_temp_message("Cannot load hand.")















