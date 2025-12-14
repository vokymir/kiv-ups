import queue
from . import config
from .ui import PrsiApp
from .net import NetworkManager
from tkinter import messagebox

class GameClient:
    """
    The 'Brain' of the application.
    Mediates between the Network Thread and the UI Thread.
    """
    def __init__(self):
        # 1. Thread-safe queue for incoming network messages
        self.msg_queue = queue.Queue()

        # 2. Network Manager (starts in own thread)
        self.net = NetworkManager(self.msg_queue)

        # 3. UI (runs on main thread)
        self.app = PrsiApp(self)

        # 4. Start polling the queue
        self.app.after(100, self.process_incoming_messages)

    def run(self):
        self.app.mainloop()

    # --- Outgoing Requests (UI -> Net) ---

    def request_login(self, ip, port, username):
        if not ip or not port or not username:
            messagebox.showerror("Error", "All fields are required")
            return

        if self.net.connect(ip, port, username):
            self.net.send_command(f"{config.CMD_LOGIN} {username}")
            # Wait for server confirmation in process_incoming_messages
        else:
            messagebox.showerror("Connection Failed", "Could not connect to server.")

    def request_join_room(self, room_id):
        self.net.send_command(f"{config.CMD_JOIN} {room_id}")

    def request_leave_server(self):
        self.net.send_command(config.CMD_LEAVE_SERVER)
        self.net.disconnect()
        self.app.show_frame("LoginFrame")

    def request_leave_room(self):
        self.net.send_command(config.CMD_LEAVE_ROOM)
        # Optimistically switch back to lobby
        self.app.show_frame("LobbyFrame")

    def request_play_card(self, card_name):
        print(f"Playing card: {card_name}")
        self.net.send_command(f"PLAY {card_name}")

    def request_draw_card(self):
        print("Drawing card")
        self.net.send_command("DRAW")

    # --- Incoming Handling (Net -> UI) ---

    def process_incoming_messages(self):
        """
        Periodically checks the queue for messages from the network thread.
        This runs on the MAIN UI THREAD, so it is safe to update Tkinter.
        """
        while not self.msg_queue.empty():
            msg_type, content = self.msg_queue.get()

            if msg_type == "ERROR":
                messagebox.showerror("Network Error", content)
                self.app.show_frame("LoginFrame")

            elif msg_type == "DISCONNECTED":
                self.app.show_frame("LoginFrame")

            elif msg_type == "SERVER_MSG":
                self.handle_protocol(content)

        # Schedule next check in 100ms
        self.app.after(100, self.process_incoming_messages)

    def handle_protocol(self, line):
        """
        Parses the custom text protocol from C++.
        Implement your parsing logic here.
        """
        print(f"[PROTO] Received: {line}")
        parts = line.split()
        cmd = parts[0]

        # --- DUMMY IMPLEMENTATION FOR DEMONSTRATION ---

        # Example: Server says "LOGIN_OK" -> Go to Lobby
        if cmd == "LOGIN_OK":
            self.app.show_frame("LobbyFrame")
            # For testing: Populate dummy rooms
            dummy_rooms = [
                {'id': 1, 'state': 'OPEN', 'players': '1/2'},
                {'id': 2, 'state': 'PLAYING', 'players': '2/2'},
                {'id': 3, 'state': 'OPEN', 'players': '0/2'},
            ]
            self.app.get_frame("LobbyFrame").update_rooms(dummy_rooms)

        # Example: Server says "JOIN_OK 1" -> Go to Game
        elif cmd == "JOIN_OK":
            self.app.show_frame("GameFrame")
            # Setup dummy initial game state
            game_frame = self.app.get_frame("GameFrame")
            game_frame.lbl_opponent_name.config(text="Opponent: Karel")
            game_frame.set_opponent_hand(4)
            game_frame.set_deck_visible(True)
            game_frame.set_table_card("7_hearts") # Ensure you have 7_hearts.jpg or it generates placeholder
            game_frame.update_player_hand(["ace_spades", "10_hearts", "upper_green", "king_balls"])

        # Example: Server says "UPDATE_HAND ace_spades 7_hearts"
        elif cmd == "UPDATE_HAND":
            cards = parts[1:]
            self.app.get_frame("GameFrame").update_player_hand(cards)

        # Example: Server says "OPPONENT_CARD_COUNT 5"
        elif cmd == "OPPONENT_CARD_COUNT":
            count = int(parts[1])
            self.app.get_frame("GameFrame").set_opponent_hand(count)
