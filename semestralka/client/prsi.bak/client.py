import queue
from tkinter import messagebox
from typing import Any, List, Dict, Tuple, Optional
from queue import Queue
# Changed to relative imports
from . import config
from .ui import PrsiApp, GameFrame
from .net import NetworkManager, QueueMessage # Importing QueueMessage type

class GameClient:
    """
    The 'Brain' of the application.
    Mediates between the Network Thread and the UI Thread.
    """
    def __init__(self) -> None:
        # 1. Thread-safe queue for incoming network messages
        self.msg_queue: Queue[QueueMessage] = queue.Queue()

        # 2. Network Manager (starts in own thread)
        self.net: NetworkManager = NetworkManager(self.msg_queue)

        # 3. UI (runs on main thread)
        self.app: PrsiApp = PrsiApp(self)

        # 4. Start polling the queue
        self.app.after(100, self.process_incoming_messages)

    def run(self) -> None:
        self.app.mainloop()

    # --- Outgoing Requests (UI -> Net) ---

    def request_login(self, ip: str, port: str, username: str) -> None:
        if not ip or not port or not username:
            messagebox.showerror("Error", "All fields are required")
            return

        if self.net.connect(ip, port, username):
            self.net.send_command(f"{config.CMD_LOGIN} {username}")
            # Wait for server confirmation in process_incoming_messages
        else:
            messagebox.showerror("Connection Failed", "Could not connect to server.")

    def request_join_room(self, room_id: int) -> None:
        self.net.send_command(f"{config.CMD_JOIN} {room_id}")

    def request_leave_server(self) -> None:
        self.net.send_command(config.CMD_LEAVE_SERVER)
        self.net.disconnect()
        self.app.show_frame("LoginFrame")

    def request_leave_room(self) -> None:
        self.net.send_command(config.CMD_LEAVE_ROOM)
        # Optimistically switch back to lobby
        self.app.show_frame("LobbyFrame")

    def request_play_card(self, card_name: str) -> None:
        print(f"Playing card: {card_name}")
        self.net.send_command(f"PLAY {card_name}")

    def request_draw_card(self) -> None:
        print("Drawing card")
        self.net.send_command("DRAW")

    # --- Incoming Handling (Net -> UI) ---

    def process_incoming_messages(self) -> None:
        """
        Periodically checks the queue for messages from the network thread.
        This runs on the MAIN UI THREAD, so it is safe to update Tkinter.
        """
        while not self.msg_queue.empty():
            # msg_type is str, content is Optional[str]
            msg_type, content = self.msg_queue.get()

            if msg_type == "ERROR":
                messagebox.showerror("Network Error", content if content else "Unknown Error")
                self.app.show_frame("LoginFrame")

            elif msg_type == "DISCONNECTED":
                self.app.show_frame("LoginFrame")

            elif msg_type == "SERVER_MSG" and content is not None:
                self.handle_protocol(content)

        # Schedule next check in 100ms
        self.app.after(100, self.process_incoming_messages)

    def handle_protocol(self, line: str) -> None:
        """
        Parses the custom text protocol from C++.
        Implement your parsing logic here.
        """
        print(f"[PROTO] Received: {line}")
        parts: List[str] = line.split()
        if not parts:
            return

        cmd: str = parts[0]

        # --- DUMMY IMPLEMENTATION FOR DEMONSTRATION ---

        # Example: Server says "LOGIN_OK" -> Go to Lobby
        if cmd == "LOGIN_OK":
            self.app.show_frame("LobbyFrame")
            # For testing: Populate dummy rooms
            dummy_rooms: List[Dict[str, Any]] = [
                {'id': 1, 'state': 'OPEN', 'players': '1/2'},
                {'id': 2, 'state': 'PLAYING', 'players': '2/2'},
                {'id': 3, 'state': 'OPEN', 'players': '0/2'},
            ]
            lobby_frame = self.app.get_frame("LobbyFrame")
            # Ensure lobby_frame is the expected type before calling update_rooms
            if hasattr(lobby_frame, 'update_rooms'):
                 lobby_frame.update_rooms(dummy_rooms)

        # Example: Server says "JOIN_OK 1" -> Go to Game
        elif cmd == "JOIN_OK":
            self.app.show_frame("GameFrame")
            # Setup dummy initial game state
            game_frame: Any = self.app.get_frame("GameFrame")

            if isinstance(game_frame, GameFrame):
                game_frame.lbl_opponent_name.config(text="Opponent: Karel")
                game_frame.set_opponent_hand(4)
                game_frame.set_deck_visible(True)
                game_frame.set_table_card("7_hearts") # Ensure you have 7_hearts.jpg or it generates placeholder
                game_frame.update_player_hand(["ace_spades", "10_hearts", "upper_green", "king_balls"])

        # Example: Server says "UPDATE_HAND ace_spades 7_hearts"
        elif cmd == "UPDATE_HAND":
            cards: List[str] = parts[1:]
            game_frame_update: Any = self.app.get_frame("GameFrame")
            if isinstance(game_frame_update, GameFrame):
                game_frame_update.update_player_hand(cards)

        # Example: Server says "OPPONENT_CARD_COUNT 5"
        elif cmd == "OPPONENT_CARD_COUNT":
            if len(parts) > 1:
                try:
                    count: int = int(parts[1])
                    game_frame_count: Any = self.app.get_frame("GameFrame")
                    if isinstance(game_frame_count, GameFrame):
                        game_frame_count.set_opponent_hand(count)
                except ValueError:
                    print(f"[PROTO] Error: Invalid card count received: {parts[1]}")
