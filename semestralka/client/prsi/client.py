from datetime import datetime, timedelta, timezone
from queue import Queue
from tkinter import messagebox
import queue

try:
    # Python >= 3.12
    from typing import override
except ImportError:
    # Python <= 3.11
    from typing_extensions import override

from prsi.net import QM_DISCONNECTED, QM_ERROR, QM_MESSAGE, Net, Queue_Message
from prsi.ui import Ui
from prsi.config import CMD_CREATE_ROOM, CMD_DRAW, CMD_JOIN, CMD_LEAVE_ROOM, CMD_NAME, CMD_PLAY, CMD_PONG, CMD_ROOM, CMD_ROOMS, CMD_STATE, FN_LOBBY, FN_LOGIN, FN_ROOM, PROTO_DELIM, ST_GAME, ST_LOBBY, ST_ROOM, ST_UNNAMED
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
        # to remember what player did & remove from hand
        self.last_played: Card | None = None

        # Already Sent NAME/DRAW/PLAY
        self.already_sent: bool = False
        # Already sent join/create/list rooms
        self.lobby_tried: bool = False

        # reconnect stuff
        self.last_ping_recv: datetime = datetime.now(timezone.utc)
        self.timeout_sleep: timedelta = timedelta(seconds=7)
        self.timeout_dead: timedelta = timedelta(seconds=120)
        self.notified_server_inactivity: bool = False
        # track manual disconnects
        self.manual_disconnect: bool = False

        # network part of client - talk via queue
        self.net: Net = Net(self.mq)

        # ui
        self.ui: Ui = Ui(self)

        self.check_server_availability()
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

    # reconnect stuff
    def check_server_availability(self) -> None:
        """
        Periodically check if server is alive.
        Auto-reconnect if temporarily unreachable.
        Leave server only after timeout_dead.
        """
        now: datetime = datetime.now(timezone.utc)
        elapsed: timedelta = now - self.last_ping_recv

        if elapsed > self.timeout_dead:
            # Server dead: leave server for real
            if self.player:
                self.ui.show_info_window("Server is not available. Leaving...")
            self.disconnect(manual=True)  # now it's final disconnect

        elif elapsed > self.timeout_sleep:
            # Server temporarily unreachable: notify, try reconnect
            if not self.notified_server_inactivity:
                self.notified_server_inactivity = True
                self.ui.show_info_window("Server seems unreachable. Trying to reconnect...")

            if self.player:
                # attempt reconnect without losing player
                p = Player(self.player.nick)
                p.ip = self.player.ip
                p.port = self.player.port
                self.disconnect(manual=False)  # temporary disconnect, keep UI
                self.player = p
                try:
                    self.connect(p.ip, int(p.port), p.nick)
                except Exception:
                    # still unreachable: will retry on next check
                    pass

        else:
            # server reachable, reset notification flag
            self.notified_server_inactivity = False

        # check again later
        _ = self.ui.after(2000, self.check_server_availability)  # check every 2 seconds

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

    @override
    def get_room(self) -> Room | None:
        if (self.room):
            return self.room
        return None

    # ui -> net

    # == any time

    @override
    def disconnect(self, manual: bool = True) -> None:
        """
        Disconnect from the server.
        If manual=True, the user actively left or server dead → show login.
        If manual=False, temporary disconnect (trying auto-reconnect) → keep UI as is.
        """
        self.net.disconnect()

        self.room = None
        self.known_rooms_ = []

        if manual:
            self.player = None
            self.ui.switch_frame(FN_LOGIN)

    @override
    def state(self) -> None:
        """
        to get the state of player
        """
        self.net.send_command(CMD_STATE)

    # == unnamed

    @override
    def connect(self, ip: str, port: int, username: str) -> None:
        self.manual_disconnect = False  # allow auto reconnect again
        self.last_ping_recv = datetime.now(timezone.utc)
        if (self.already_sent):
            self.ui.show_temp_message("Already trying to connect server.")
            return

        if any((ch.isspace() or ch == PROTO_DELIM) for ch in username):
            self.ui.show_info_window("Username cannot contain whitespaces.")
            self.already_sent = False
            return

        self.already_sent = True
        if not(self.net.connect(ip, str(port))):
            self.ui.show_temp_message("Cannot connect to the server")
            self.already_sent = False
            return

        self.already_sent = False
        self.net.send_command(CMD_NAME + " " + username)
        # save username here - after dont have it
        self.player = Player(username)
        self.player.ip = ip
        self.player.port = str(port)
        self.ui.show_temp_message("Trying to connect.", 1000)

    # == lobby

    @override
    def rooms(self) -> None:
        """
        Ask server for rooms
        AS=none, State=Lobby
        """
        if ((not self.lobby_tried) and self.player and self.player.state == ST_LOBBY):
            self.net.send_command(CMD_ROOMS)
            self.lobby_tried = True
            return
        self.ui.show_temp_message("Cannot show rooms when not in lobby")

    @override
    def create_room(self) -> None:
        if ((not self.lobby_tried) and self.player and self.player.state == ST_LOBBY):
            self.net.send_command(CMD_CREATE_ROOM)
            self.lobby_tried = True
            return
        self.ui.show_temp_message("Cannot create room when not in lobby")

    @override
    def join(self, room_id: int) -> None:
        """
        AS=none, State=Lobby
        """
        if ((not self.lobby_tried) and self.player and self.player.state == ST_LOBBY):
            self.net.send_command(CMD_JOIN + " " + str(room_id))
            self.lobby_tried = True
            return
        self.ui.show_temp_message("Cannot join room when not in lobby")

    # == room

    @override
    def room_info(self) -> None:
        if (self.room):
            self.net.send_command(CMD_ROOM)
        else:
            self.net.send_command(CMD_STATE)

    @override
    def leave_room(self) -> None:
        if (self.player and (self.player.state == ST_ROOM or \
                             self.player.state == ST_GAME)):
            self.net.send_command(CMD_LEAVE_ROOM)
            self.player.state = ST_LOBBY
            self.player.hand = []
            self.room = None
            self.net.send_command(CMD_ROOMS)

    # == game

    @override
    def draw_card(self) -> None:
        """
        AS=true, State=game
        """
        if (self.room and self.room.turn and (not self.already_sent)):
            self.net.send_command(CMD_DRAW)
            self.already_sent = True
            return
        self.ui.show_temp_message("It's not your turn.")

    @override
    def play_card(self, card: Card) -> None:
        if (self.room and self.room.turn and (not self.already_sent)):
            if (card.rank == "Q" or\
                card.rank == self.room.top_card.rank or\
                card.suit == self.room.top_card.suit):
                self.net.send_command(CMD_PLAY + " " + card.__str__())
                self.last_played = card
                self.already_sent = True
            else:
                self.ui.show_temp_message("You cannot play this.")
        else:
            self.ui.show_temp_message("You cannot play.")



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
                self.parse_ok_message(parts)
            case "FAIL":
                self.ui.show_temp_message(f"ERROR: {msg}",10000)
            case "ROOMS":
                self.parse_rooms_message(parts)
                self.ui.refresh_lobby()
                self.ui.switch_frame(FN_LOBBY)
            case "ROOM":
                _ = self.parse_room_message(parts)
            case "GAME_START":
                self.parse_gamestart_message(parts)
            case "HAND":
                _ = self.parse_hand_message(parts)
            case "TURN":
                self.parse_turn_message(parts)
            case "PLAYED":
                self.parse_played_message(parts)
            case "DRAWED":
                self.parse_drawed_message(parts)
            case "SKIP":
                self.parse_skip_message(parts)
            case "CARDS":
                self.parse_cards_message(parts)
                if (self.player):
                    self.ui.room_frame.update_hand(self.player.hand)
            case "WIN":
                self.parse_win_message(parts)
                # show you are winner
            case "LOSE":
                self.parse_lose_message(parts)
                # show you are loser
            case "PING":
                self.parse_ping_message()
            case "STATE":
                self.parse_state_message(parts)
                # change UI accordingly
            case "SLEEP":
                self.parse_sleep_message(parts)
                # show who is sleeping
            case "DEAD":
                self.parse_dead_message(parts)
                # show who is dead
            case "AWAKE":
                self.parse_awake_message(parts)
                # show who is awake
            case "JOIN":
                pass # to get room info is called elsewhere
            case "LEAVE":
                pass # the win message will follow
            case _:
                self.ui.show_temp_message(f"Received unknown message from server: {msg}")

    def parse_ping_message(self) -> None:
        now: datetime = datetime.now(timezone.utc)
        elapsed: timedelta = now - self.last_ping_recv

        if (self.net.connected and elapsed > self.timeout_sleep):
            self.notified_server_inactivity = False
            self.net.send_command(CMD_STATE) # ask whats new
            self.ui.show_info_window("Server is available.")

        self.last_ping_recv = now
        self.net.send_command(CMD_PONG)

    def parse_ok_message(self, msg: list[str]) -> None:
        if (len(msg) > 1):
            match msg[1]:
                case "NAME":
                    self.already_sent = False

                    if (not self.player):
                        self.ui.show_temp_message("WEIRD BUG #1")
                        self.disconnect()
                    else:
                        self.player.state = ST_LOBBY
                    self.net.send_command(CMD_STATE)
                case "JOIN_ROOM":
                    self.net.send_command(CMD_ROOM)
                case "CREATE_ROOM":
                    self.net.send_command(CMD_ROOM)
                case "PLAY":
                    if (self.player and self.last_played):
                        self.player.discard(self.last_played)
                        self.ui.room_frame.update_hand(self.player.hand)
                    self.last_played = None

                case _:
                    pass

        return


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
            self.lobby_tried = False

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid rooms message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Cannot display rooms.")

    def parse_room_message(self, msg: list[str]) -> int:
        try:
            n_tokens: int = 0
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

            # total number of parsed tokens
            n_tokens = 5 + n_players * 3

            # set global my room
            if (self.room ):
                # always remember
                room.turn = self.room.turn
                # only remember top card if new is invalid
                if (room.top_card.suit == "N" or\
                    room.top_card.rank == "N"):
                    room.top_card = self.room.top_card

            self.room = room

            # if not in game, set the state to room
            if (self.player and self.player.state != ST_GAME):
                self.player.state = ST_ROOM

            # UI
            if (not self.room):
                self.ui.show_temp_message("Cannot retrieve room info")
            self.ui.room_frame.set_room_id()
            self.ui.room_frame.draw_opponent_cards(self.opponent_n_cards())
            op: Player | None = self.opponent()
            if (isinstance(op, Player)):
                self.ui.room_frame.draw_opponent_name(op.nick)
            self.ui.switch_frame(FN_ROOM)

            self.lobby_tried = False
            return n_tokens

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid room message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Cannot display room.")
            return -1


    def parse_hand_message(self, msg: list[str]) -> int:
        try:
            n_tokens: int = 0
            count: int = int(msg[1])
            hand: list[Card] = []

            for i in range(2, 2+count):
                cd: str = msg[i]
                suit: str = cd[0]
                rank: str = cd[1]

                card: Card = Card(suit, rank)
                hand.append(card)

            n_tokens = 2 + count

            if (self.player):
                self.player.hand = hand

            # UI
            if (self.player):
                self.ui.room_frame.update_hand(self.player.hand)

            return n_tokens

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid hand message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Cannot load hand.")
            return -1

    def parse_turn_message(self, msg: list[str]) -> None:
        try:
            name: str = msg[1]
            top: str = msg[3]

            card: Card = Card(top[0], top[1])

            if (not self.room):
                # prepare info about top card
                # inside invalid room
                self.room = Room(-1,"TEMPORARY")
                self.room.top_card = card
            else:
                self.room.top_card = card

            if (self.player and self.player.nick == name):
                self.room.turn = True
                self.ui.show_info_window("Your turn.")
            else:
                self.room.turn = False
                self.ui.show_temp_message("Opponents turn")

            # definitely, its another turn
            self.already_sent = False

            # UI
            self.ui.room_frame.update_pile()

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid turn message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Don't know whose turn it is")

    def parse_gamestart_message(self, _msg: list[str]) -> None:
        if self.player:
            self.player.state = ST_GAME

        self.already_sent = False

        self.net.send_command(CMD_ROOM)

    def parse_played_message(self, msg: list[str]) -> None:
        try:
            card: Card = Card(msg[2][0], msg[2][1])

            if (self.room):
                self.room.top_card = card

            # assume there is only one opponent
            op: Player | None = self.opponent()
            if (op):
                op.n_cards -= 1
                self.ui.room_frame.draw_opponent_cards(op.n_cards)

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid played message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Someone played a card?")


    def parse_drawed_message(self, msg: list[str]) -> None:
        try:
            who: str = msg[1]
            count: int = int(msg[2])

            opp: Player | None = self.opponent()
            if (opp and opp.nick == who):
                if (self.room):
                    opp.n_cards += count

                    self.ui.room_frame.draw_opponent_cards(opp.n_cards)

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid drawed message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Someone drawed a card?")

    def parse_skip_message(self, msg: list[str]) -> None:
        try:
            who: str = msg[1]
            self.ui.show_temp_message(f"Player {who} was skipped.")

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid skip message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Someone was skipped?")

    def parse_cards_message(self, msg: list[str]) -> None:
        try:
            count: int = int(msg[1])
            cards: list[Card] = []

            for i in range(2, 2+count):
                cd: str = msg[i]
                suit: str = cd[0]
                rank: str = cd[1]

                card: Card = Card(suit, rank)
                cards.append(card)

            if (self.player):
                self.player.hand.extend(cards)

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid skip message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Someone was skipped?")

    def parse_win_message(self, _msg: list[str]) -> None:
        if (self.player):
            self.player.hand = []
            self.ui.room_frame.update_hand([])
        self.ui.show_info_window("You won gracefully.");
        if (self.room): # avoid player clicking on stuff
            self.room.turn = False

    def parse_lose_message(self, _msg: list[str]) -> None:
        if (self.player):
            self.player.hand = []
            self.ui.room_frame.update_hand([])
        self.ui.show_info_window("You've been defeated.");
        if (self.room): # avoid player clicking on stuff
            self.room.turn = False

    def parse_state_message(self, msg: list[str]) -> None:
        try:
            _ = msg.pop(0) # so the whole msg could be passed to room message
            state: str = msg.pop(0)

            if (state == ST_UNNAMED):
                self.ui.switch_frame(FN_LOGIN)

            elif (state == ST_LOBBY):
                self.net.send_command(CMD_ROOMS)
                self.ui.switch_frame(FN_LOBBY)

            elif (state == ST_ROOM):
                _ = self.parse_room_message(msg)

            elif (state == ST_GAME):
                n_tokens: int = self.parse_room_message(msg)

                if (n_tokens < 0): raise Exception("Incorrect room message.")
                for i in range(n_tokens):
                    _ = msg.pop(0)

                n_tokens = self.parse_hand_message(msg)

                if (n_tokens < 0): raise Exception("Incorrect hand message.")
                for i in range(n_tokens):
                    _ = msg.pop(0)

                self.parse_turn_message(msg)

            else:
                self.ui.show_temp_message("Server error happened. \
                Please try again later.")
                self.disconnect()

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid state message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Where am I?")

    def parse_sleep_message(self, msg: list[str]) -> None:
        try:
            name: str = msg[1]

            if (self.player and self.player.state == ST_GAME):
                self.ui.show_temp_message(f"Player {name} is not connected...")

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid sleep message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Someone is asleep?")


    def parse_dead_message(self, msg: list[str]) -> None:
        try:
            name: str = msg[1]

            if (self.player and self.player.state == ST_GAME):
                self.ui.show_temp_message(f"Player {name} disconnected...")

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid dead message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Someone is dead?")

    def parse_awake_message(self, msg: list[str]) -> None:
        try:
            name: str = msg[1]

            if (self.player and self.player.state == ST_GAME):
                self.ui.show_temp_message(f"Player {name} is online.")

        except Exception as e:
            joined: str = " ".join(msg)
            print(f"[PROTO] invalid awake message received ({joined})\
            resulting in: {e}")
            self.ui.show_temp_message("Someone is alive?")





