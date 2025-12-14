import tkinter as tk
from tkinter import messagebox
import socket
import threading
import time

# --- CONFIGURATION AND CONSTANTS ---
HOST = '127.0.0.1'  # Default IP
PORT = 42690         # Default Port (Updated)
BUFFER_SIZE = 4096

# Custom Protocol Constants
MAGIC = "PRSI"
DELIM = "|"

# Suits: 0: Hearts(♥, Red), 1: Diamonds(♦, Red), 2: Clubs(♣, Black), 3: Spades(♠, Black)
SUITS = [
    {'symbol': '♥', 'color': 'red', 'name': 'Hearts'},
    {'symbol': '♦', 'color': 'red', 'name': 'Diamonds'},
    {'symbol': '♣', 'color': 'black', 'name': 'Clubs'},
    {'symbol': '♠', 'color': 'black', 'name': 'Spades'}
]
RANKS = ['7', '8', '9', '10', 'J', 'Q', 'K', 'A']

# --- MAIN APPLICATION CLASS ---

class PrsiClientApp:
    def __init__(self, master):
        self.master = master
        master.title("Prší TCP Client")
        master.geometry("800x600")
        master.configure(bg="#1a1a2e")

        self.socket = None
        self.username = ""
        self.user_id = "" # In a real app, the server would assign this
        self.current_room_id = None
        self.game_state = {
            'opponentName': 'Waiting...',
            'opponentCardCount': 0,
            'topCard': None,
            'hand': []
        }

        # Setup main frames (Screens)
        self.frames = {}
        self._setup_login_frame()
        self._setup_lobby_frame()
        self._setup_game_frame()

        # Initial screen
        self.show_frame("login")

        # Handle closing the window
        master.protocol("WM_DELETE_WINDOW", self.on_close)

    def show_frame(self, name):
        """Displays the requested frame and hides others."""
        for frame in self.frames.values():
            frame.grid_forget()
        self.frames[name].grid(row=0, column=0, sticky="nsew")

    # --- UI SETUP ---

    def _setup_login_frame(self):
        """Sets up the IP/Port/Username input screen."""
        frame = tk.Frame(self.master, bg="#1a1a2e", padx=50, pady=50)
        self.frames["login"] = frame

        # Centering configuration
        frame.grid_rowconfigure(0, weight=1)
        frame.grid_rowconfigure(2, weight=1)
        frame.grid_columnconfigure(0, weight=1)

        login_card = tk.Frame(frame, bg="#16213e", padx=30, pady=30, bd=5, relief=tk.RIDGE)
        login_card.grid(row=1, column=0)

        tk.Label(login_card, text="PRŠÍ ONLINE", font=("Arial", 24, "bold"), fg="#e94560", bg="#16213e").pack(pady=10)

        # IP Field
        tk.Label(login_card, text="Server IP:", fg="#e0e0e0", bg="#16213e", anchor='w').pack(fill='x', pady=(10, 0))
        self.ip_entry = tk.Entry(login_card, width=30, bd=0, highlightthickness=0, bg="#0f3460", fg="white", insertbackground="white")
        self.ip_entry.insert(0, HOST)
        self.ip_entry.pack(pady=5)

        # Port Field
        tk.Label(login_card, text="Port:", fg="#e0e0e0", bg="#16213e", anchor='w').pack(fill='x', pady=(10, 0))
        self.port_entry = tk.Entry(login_card, width=30, bd=0, highlightthickness=0, bg="#0f3460", fg="white", insertbackground="white")
        self.port_entry.insert(0, str(PORT))
        self.port_entry.pack(pady=5)

        # Username Field
        tk.Label(login_card, text="Username:", fg="#e0e0e0", bg="#16213e", anchor='w').pack(fill='x', pady=(10, 0))
        self.username_entry = tk.Entry(login_card, width=30, bd=0, highlightthickness=0, bg="#0f3460", fg="white", insertbackground="white")
        self.username_entry.insert(0, "Player")
        self.username_entry.pack(pady=5)

        # Connect Button
        tk.Button(login_card, text="Connect", command=self.connect, bg="#e94560", fg="white", relief=tk.FLAT, activebackground="#c93550", activeforeground="white").pack(pady=20, fill='x')

    def _setup_lobby_frame(self):
        """Sets up the Lobby screen with room listing."""
        frame = tk.Frame(self.master, bg="#1a1a2e", padx=20, pady=20)
        self.frames["lobby"] = frame
        frame.grid_columnconfigure(0, weight=1)
        frame.grid_rowconfigure(1, weight=1)

        # Header
        header = tk.Frame(frame, bg="#1a1a2e")
        header.grid(row=0, column=0, sticky="ew", pady=(0, 10))
        tk.Label(header, text="Game Lobby", font=("Arial", 20, "bold"), fg="#e0e0e0", bg="#1a1a2e").pack(side=tk.LEFT)
        self.user_label = tk.Label(header, text="Logged in as: N/A", fg="#888", bg="#1a1a2e")
        self.user_label.pack(side=tk.LEFT, padx=15)
        tk.Button(header, text="Logout", command=self.disconnect, bg="#e84118", fg="white", relief=tk.FLAT, activebackground="#b83108").pack(side=tk.RIGHT)

        # Room List Area (Canvas for scrollability)
        canvas = tk.Canvas(frame, bg="#1a1a2e", highlightthickness=0)
        canvas.grid(row=1, column=0, sticky="nsew")

        scrollbar = tk.Scrollbar(frame, orient="vertical", command=canvas.yview)
        scrollbar.grid(row=1, column=1, sticky="ns")
        canvas.configure(yscrollcommand=scrollbar.set)

        self.room_list_frame = tk.Frame(canvas, bg="#1a1a2e")
        canvas.create_window((0, 0), window=self.room_list_frame, anchor="nw", width=750)

        self.room_list_frame.bind("<Configure>", lambda e: canvas.configure(scrollregion = canvas.bbox("all")))

        self.frames["lobby_canvas"] = canvas # Store canvas for later updates

    def _setup_game_frame(self):
        """Sets up the main Game screen."""
        frame = tk.Frame(self.master, bg="#0f0f1a")
        self.frames["game"] = frame
        frame.grid_rowconfigure(1, weight=1)
        frame.grid_columnconfigure(0, weight=1)

        # 1. Top Bar (Opponent Info)
        top_bar = tk.Frame(frame, bg="#151525", height=80)
        top_bar.grid(row=0, column=0, sticky="ew")
        top_bar.grid_columnconfigure(0, weight=1)
        top_bar.grid_columnconfigure(2, weight=1)

        self.opp_name_label = tk.Label(top_bar, text="Opponent: Waiting...", font=("Arial", 14), fg="#e0e0e0", bg="#151525")
        self.opp_name_label.grid(row=0, column=0, padx=20, pady=10, sticky='w')

        opp_cards_frame = tk.Frame(top_bar, bg="#151525")
        opp_cards_frame.grid(row=0, column=1, padx=20, pady=10)

        tk.Label(opp_cards_frame, text="Opponent Cards: ", fg="#e0e0e0", bg="#151525").pack(side=tk.LEFT)
        self.opp_count_label = tk.Label(opp_cards_frame, text="0", font=("Arial", 16, "bold"), fg="#e94560", bg="#151525")
        self.opp_count_label.pack(side=tk.LEFT)

        tk.Button(top_bar, text="Leave Room", command=self.leave_room, bg="#e84118", fg="white", relief=tk.FLAT, activebackground="#b83108").grid(row=0, column=2, padx=20, pady=10, sticky='e')

        # 2. Center Board (Deck and Discard)
        board_frame = tk.Frame(frame, bg="#1b262c")
        board_frame.grid(row=1, column=0, sticky="nsew")
        board_frame.grid_columnconfigure(0, weight=1)
        board_frame.grid_rowconfigure(0, weight=1)

        self.deck_and_discard = tk.Frame(board_frame, bg="#1b262c")
        self.deck_and_discard.pack(expand=True)

        # Deck Stack - Draw Card action now uses the text protocol
        self.deck_canvas = tk.Canvas(self.deck_and_discard, width=80, height=120, bg="#0f0f1a", highlightthickness=0, bd=0)
        self.deck_canvas.pack(side=tk.LEFT, padx=30)
        self.deck_canvas.create_rectangle(5, 5, 75, 115, fill="#b71540", outline="#fff", width=3)
        self.deck_canvas.create_text(40, 60, text="DRAW", fill="white", font=("Arial", 10, "bold"))
        self.deck_canvas.bind("<Button-1>", lambda e: self.send_message("DRAW_CARD"))

        # Discard Pile (Top Card)
        self.discard_frame = tk.Frame(self.deck_and_discard, width=80, height=120, bg="#1b262c")
        self.discard_frame.pack(side=tk.LEFT, padx=30)
        self.discard_frame.pack_propagate(False)
        self.top_card_label = tk.Label(self.discard_frame, text="Discard Pile", bg="#1b262c", fg="#aaa")
        self.top_card_label.pack(expand=True)

        # 3. Bottom Hand (Player Cards)
        hand_container = tk.Frame(frame, bg="#151525", height=180, padx=10, pady=10)
        hand_container.grid(row=2, column=0, sticky="ew")

        self.hand_frame = tk.Frame(hand_container, bg="#151525")
        self.hand_frame.pack(expand=True)

        # Create 9 card slots (Labels/Frames acting as slots)
        self.card_slot_frames = []
        for i in range(9):
            slot_frame = tk.Frame(self.hand_frame, width=80, height=120, bg="#16213e", relief=tk.GROOVE, bd=2)
            slot_frame.pack(side=tk.LEFT, padx=5)
            slot_frame.pack_propagate(False)
            self.card_slot_frames.append(slot_frame)
            tk.Label(slot_frame, text="Slot", bg="#16213e", fg="#555").pack(expand=True)


    # --- NETWORK AND THREADING ---

    def connect(self):
        """Establishes TCP connection and starts listening thread."""
        ip = self.ip_entry.get()
        try:
            port = int(self.port_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Invalid port number.")
            return

        self.username = self.username_entry.get()
        if not self.username:
            messagebox.showerror("Error", "Please enter a username.")
            return

        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5)
            self.socket.connect((ip, port))
            self.socket.settimeout(None) # Remove timeout for continuous listening
        except socket.error as e:
            messagebox.showerror("Connection Error", f"Could not connect to {ip}:{port}\nError: {e}")
            return

        self.user_label.config(text=f"Logged in as: {self.username}")

        # Start the listening thread
        self.listener_thread = threading.Thread(target=self._listen_for_messages)
        self.listener_thread.daemon = True
        self.listener_thread.start()

        # Send initial login message using the text protocol
        self.send_message("NAME", self.username)

    def disconnect(self):
        """Closes the socket and returns to login screen."""
        if self.socket:
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
                self.socket.close()
            except OSError:
                pass # Socket already closed
            self.socket = None
        self.show_frame("login")

    def send_message(self, *parts):
        """Sends a message formatted as PRSI <parts>| over the TCP socket."""
        if self.socket:
            try:
                # 1. Join parts with space (e.g., "NAME Player1")
                data_string = " ".join(map(str, parts))
                # Applying the user's requested format: " PRSI <data_string> | "
                message = f" {MAGIC} {data_string} {DELIM} "
                print(f"Sending: {message.strip()}")
                self.socket.sendall(message.encode('utf-8'))
            except socket.error as e:
                self._handle_socket_error(f"Error sending data: {e}")

    def _listen_for_messages(self):
        """Thread function to continuously listen for data from the server, delimited by '|'."""
        buffer = ""
        while self.socket:
            try:
                data = self.socket.recv(BUFFER_SIZE).decode('utf-8')
                if not data:
                    self._handle_socket_error("Connection closed by server.")
                    break

                buffer += data
                while DELIM in buffer:
                    message, buffer = buffer.split(DELIM, 1)

                    # Strip leading/trailing whitespace
                    message = message.strip()

                    # Remove MAGIC prefix if present (it should be)
                    if message.startswith(MAGIC):
                        message = message[len(MAGIC):].strip()
                    elif message.startswith(f" {MAGIC} "):
                        message = message[len(MAGIC) + 2:].strip()


                    if message:
                        self.master.after(0, lambda m=message: self._handle_server_message(m))

            except socket.timeout:
                continue
            except (socket.error, ConnectionResetError) as e:
                self._handle_socket_error(f"Network error: {e}")
                break
            except Exception as e:
                print(f"Unexpected error in listener thread: {e}")
                time.sleep(1)

    def _handle_socket_error(self, message):
        """Handles connection errors, cleaning up and notifying the user."""
        self.disconnect()
        self.master.after(0, lambda: messagebox.showerror("Connection Lost", message))

    # --- SERVER MESSAGE HANDLING ---

    def _handle_server_message(self, raw_data):
        """Parses and handles incoming server text protocol messages."""
        # raw_data is now a string like "OK NAME" or "ROOMS <count> <id1> <state1> <id2> <state2> ..."

        parts = raw_data.split()
        if not parts:
            print("Received empty message.")
            return

        msg_type = parts[0]

        # Handle PING/PONG heartbeat
        if msg_type == 'PING':
            self.send_message("PONG")
            return

        if msg_type == 'OK':
            # Format: OK NAME
            if len(parts) > 1 and parts[1] == 'NAME':
                self.master.after(0, lambda: messagebox.showinfo("Connected", "Successfully logged in. Requesting rooms..."))
                # Send message to get room list
                self.send_message("LIST_ROOMS")

        elif msg_type == 'ROOMS':
            # Format: ROOMS <count> <id1> <state1> <id2> <state2> ...
            rooms = []
            # Must have at least ROOMS, COUNT, ID1, STATE1 (4 parts total)
            if len(parts) >= 4:
                try:
                    # parts[1] is the count, room data starts at parts[2]
                    i = 2
                    # Iterate in steps of 2 (ID then STATE)
                    while i + 1 < len(parts):
                        room_id_str = parts[i]
                        room_status = parts[i+1]

                        try:
                            room_id = int(room_id_str)
                            status = room_status.lower()

                            rooms.append({
                                'id': room_id,
                                'status': status,
                                'players': 1 if status == 'open' else (2 if status == 'playing' else 0) # Placeholder logic
                            })
                            i += 2 # Move past ID and STATUS
                        except ValueError:
                            print(f"Protocol error: Invalid room ID or status near index {i} in ROOMS message. Skipping pair.")
                            i += 2 # Try to skip the invalid pair

                except Exception as e:
                    print(f"Error parsing ROOMS message body: {e}")

            self.render_lobby(rooms)
            self.show_frame("lobby")

        elif msg_type == 'ERROR':
            message = " ".join(parts[1:]) if len(parts) > 1 else "An unknown server error occurred."
            messagebox.showerror("Server Error", message)

        else:
            # Placeholder for future game-specific messages (JOIN_SUCCESS, GAME_UPDATE, etc.)
            print(f"Received game or unknown message: {raw_data}")

    # --- LOBBY LOGIC ---

    def render_lobby(self, rooms):
        """Populates the room list in the lobby frame."""
        # Clear existing widgets
        for widget in self.room_list_frame.winfo_children():
            widget.destroy()

        if not rooms:
            tk.Label(self.room_list_frame, text="No active rooms.", fg="#aaa", bg="#1a1a2e", font=("Arial", 14)).pack(pady=50)
            return

        for room in rooms:
            status = room.get('status', 'unknown')
            room_id = room.get('id', 'N/A')
            players = room.get('players', 0)

            # Determine color based on status
            status_color = "#4cd137" if status == 'open' else ("#fbc531" if status == 'playing' else "#e84118")

            # Room Card Frame
            card = tk.Frame(self.room_list_frame, bg="#16213e", padx=15, pady=15, bd=1, relief=tk.RAISED)
            card.pack(fill='x', pady=5, padx=10)

            # Details
            tk.Label(card, text=f"Room #{room_id}", font=("Arial", 16, "bold"), fg="#e0e0e0", bg="#16213e").pack(side=tk.LEFT, padx=(0, 20))
            tk.Label(card, text=status.upper(), font=("Arial", 10, "bold"), fg=status_color, bg="#16213e").pack(side=tk.LEFT)
            tk.Label(card, text=f"Players: {players}/2", fg="#888", bg="#16213e").pack(side=tk.LEFT, padx=20)

            if status == 'open':
                action_button = tk.Button(card, text="Join", command=lambda r_id=room_id: self.join_room(r_id),
                                          bg="#e94560", fg="white", relief=tk.FLAT, activebackground="#c93550")
            else:
                action_button = tk.Label(card, text="Full/Busy", fg="#aaa", bg="#16213e")

            action_button.pack(side=tk.RIGHT)

        # Manually trigger scroll region update after widgets are added
        self.room_list_frame.update_idletasks()
        self.frames["lobby_canvas"].config(scrollregion=self.frames["lobby_canvas"].bbox("all"))

    def join_room(self, room_id):
        """Sends message to join the selected room."""
        # Custom protocol message for joining a room
        self.send_message("JOIN_ROOM", room_id)

    # --- GAME LOGIC AND UI UPDATES ---

    def create_card_widget(self, parent, card_data):
        """Creates a graphical representation of a single card."""
        suit_info = SUITS[card_data['suit']]

        card_widget = tk.Frame(parent, width=70, height=110, bg="white", bd=1, relief=tk.RAISED)
        card_widget.pack_propagate(False)

        # Value Label (Top)
        val_top = tk.Label(card_widget, text=card_data['rank'], fg=suit_info['color'], bg="white", font=("Arial", 12, "bold"))
        val_top.pack(anchor='nw', padx=2, pady=2)

        # Suit Label (Center)
        suit_center = tk.Label(card_widget, text=suit_info['symbol'], fg=suit_info['color'], bg="white", font=("Arial", 28))
        suit_center.pack(expand=True)

        # Value Label (Bottom - rotated text is tricky, simpler is sufficient)
        val_bottom = tk.Label(card_widget, text=card_data['rank'], fg=suit_info['color'], bg="white", font=("Arial", 12, "bold"))
        val_bottom.pack(anchor='se', padx=2, pady=2)

        return card_widget

    def update_game_ui(self):
        """Updates all game elements based on the current game_state."""
        state = self.game_state

        # Update Opponent Info
        self.opp_name_label.config(text=f"Opponent: {state['opponentName']}")
        self.opp_count_label.config(text=str(state['opponentCardCount']))

        # Update Discard Pile
        # Clear previous card
        for widget in self.discard_frame.winfo_children():
            widget.destroy()

        if state['topCard']:
            card_widget = self.create_card_widget(self.discard_frame, state['topCard'])
            card_widget.pack(expand=True)
        else:
            self.top_card_label = tk.Label(self.discard_frame, text="Discard Pile", bg="#1b262c", fg="#aaa")
            self.top_card_label.pack(expand=True)


        # Update Player Hand
        hand_cards = state['hand']
        for i, slot_frame in enumerate(self.card_slot_frames):
            # Clear slot
            for widget in slot_frame.winfo_children():
                widget.destroy()

            if i < len(hand_cards):
                card_data = hand_cards[i]
                card_widget = self.create_card_widget(slot_frame, card_data)
                card_widget.pack(expand=True)

                # Bind play action to the card widget
                # Note: Card index is crucial for the server to know which card to remove
                card_widget.bind("<Button-1>", lambda e, c=card_data, idx=i: self.play_card(c, idx))
            else:
                # Slot is empty
                tk.Label(slot_frame, text="Slot", bg="#16213e", fg="#555").pack(expand=True)

    def play_card(self, card_data, index):
        """Sends the PLAY_CARD message to the server."""
        # For text protocol, we define a simple card representation: RANK SUIT_INDEX INDEX

        # Send card data and index (for removal from hand array)
        self.send_message("PLAY_CARD", card_data['rank'], card_data['suit'], index)

    def leave_room(self):
        """Handles leaving the game room."""
        if messagebox.askyesno("Leave Room", "Are you sure you want to leave the game?"):
            # Custom protocol message for leaving
            self.send_message("LEAVE_ROOM")
            self.current_room_id = None
            self.show_frame("lobby")

    def on_close(self):
        """Clean up when the window is closed."""
        self.disconnect()
        self.master.destroy()

# --- RUN APPLICATION ---

if __name__ == "__main__":
    root = tk.Tk()
    app = PrsiClientApp(root)
    root.mainloop()
