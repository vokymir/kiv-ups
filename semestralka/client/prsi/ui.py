import os
import tkinter as tk
from tkinter import Image, messagebox

from PIL import Image, ImageTk

# hehe
from prsi.config import ACCENT_COLOR, APP_TITLE, ASSETS_DIR, BG_COLOR, CARD_BACK, CARD_BG, CARD_HEIGHT, CARD_WIDTH, DEFAULT_IP, DEFAULT_PORT, FN_LOGIN, FN_LOBBY, FN_ROOM, FONT_LARGE, FONT_MEDIUM, FONT_SMALL, PAD_X, PAD_Y, TABLE_COLOR, TEXT_COLOR, WINDOW_HEIGHT, WINDOW_WIDTH
from prsi.common import Card, Client_Dummy, Room

class Img_Loader:
    """
    Loading JPG images from assets directory.
    """
    def __init__(self, card_width: int, card_height: int, assets_dir: str) -> None:
        self.c_w: int = card_width
        self.c_h: int = card_height
        self.assets_dir: str = assets_dir
        self.images: dict[str, ImageTk.PhotoImage] = {}

        # preload back of card - to fail early
        _ = self.get_image(CARD_BACK)

    def get_image(self, card_name: str) -> ImageTk.PhotoImage:
        """
        Load the image. Card name must contain .jpg (or other) extension.
        """
        if (card_name in self.images):
            return self.images[card_name]

        try:
            # attemp to load actual image
            img_path: str = os.path.join(self.assets_dir, card_name)
            img = Image.open(img_path).resize(
                (self.c_w, self.c_h), Image.Resampling.LANCZOS)
            # save & return
            self.images[card_name] = ImageTk.PhotoImage(img)
            return self.images[card_name]

        except Exception as e:
            print(f"[IMG] Exception when loading card {card_name}: {e}")
            # return at least something
            return self.images[CARD_BACK]

class Ui(tk.Tk):
    def __init__(self, client: Client_Dummy) -> None:
        super().__init__()
        self.client: Client_Dummy = client
        self.img_loader: Img_Loader = Img_Loader(CARD_WIDTH, CARD_HEIGHT, ASSETS_DIR)
        print("[UI] Initializing Tkinter root window (Ui)...")

        self.title(APP_TITLE)
        self.geometry(f"{WINDOW_WIDTH}x{WINDOW_HEIGHT}")
        _ =self.configure(bg=BG_COLOR)
        self.minsize(WINDOW_WIDTH, WINDOW_HEIGHT)

        # root container
        _ = self.grid_columnconfigure(0, weight=1)
        _ = self.grid_rowconfigure(0, weight=1)

        # container = main frame for screens, fills the root container
        container: tk.Frame = tk.Frame(self, bg=BG_COLOR)
        container.grid(row=0, column=0, sticky="nsew")

        # holder for all frames
        self.frames: dict[str, tk.Frame] = {}
        self.login_frame: Login_Screen = Login_Screen(container, self, client)
        self.lobby_frame: Lobby_Screen = Lobby_Screen(container, self, client)
        self.room_frame: Game_Screen = Game_Screen(container, self, client)

        self.frames[FN_LOGIN] = self.login_frame
        self.frames[FN_LOBBY] = self.lobby_frame
        self.frames[FN_ROOM] = self.room_frame

        for frame in self.frames.values():
            frame.grid(row=0, column=0, sticky="nsew")

        self.switch_frame(FN_LOGIN)
        print("[UI] Initialization complete. Login screen is set to be raised.")

    def switch_frame(self, frame_name: str) -> None:
        """
        Raise the selected frame to the top.
        """
        frame = self.frames.get(frame_name)
        if frame is None:
            print(f"[WARN] No frame named {frame_name}")
            return

        frame.tkraise()

    def show_temp_message(self, text: str, duration_ms: int = 3000) -> None:
        label = tk.Label(self, text=text, bg="#333", fg="white", padx=10, pady=5)
        label.place(relx=0.5, rely=0.95, anchor="s")

        _ = self.after(duration_ms, label.destroy)

    def refresh_lobby(self) -> None:
        self.lobby_frame.refresh_room_list()

class Login_Screen(tk.Frame):
    def __init__(self, parent: tk.Frame, ui_master: Ui, client: Client_Dummy) -> None:
        super().__init__(bg=BG_COLOR)
        self.ui: Ui = ui_master
        self.client: Client_Dummy = client
        print("[UI] Initializing Login Screen widgets...")

        # input variables
        self.ip_var: tk.StringVar = tk.StringVar(value=DEFAULT_IP)
        self.port_var: tk.StringVar = tk.StringVar(value=str(DEFAULT_PORT))
        self.username_var: tk.StringVar = tk.StringVar(value="Player")

        # positioning
        center_frame: tk.Frame = tk.Frame(self, bg=BG_COLOR,
                                padx=PAD_X * 3, pady=PAD_Y * 3,
                                bd=2, relief=tk.RIDGE)
        center_frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

        # widget setup
        tk.Label(center_frame, text=APP_TITLE, font=FONT_LARGE,
                bg=BG_COLOR, fg=TEXT_COLOR).grid(row=0,
                    column=0, columnspan=2, pady=PAD_Y * 2)
        fields: list[tuple[str, tk.StringVar]] = [
            ("Server IP:", self.ip_var),
            ("Port:", self.port_var),
            ("Username:", self.username_var)
        ]

        # input box setup
        for i, (label_text, var) in enumerate(fields):
            tk.Label(center_frame, text=label_text, font=FONT_MEDIUM,
                     bg=BG_COLOR, fg=TEXT_COLOR).grid(
                row=i + 1, column=1, sticky=tk.E, padx=PAD_X,
                pady=PAD_Y)
            tk.Entry(center_frame, textvariable=var, font=FONT_MEDIUM,
                     width=25, bg=CARD_BG, fg="#000000").grid(
                row=i + 1, column=1, sticky=tk.E, padx=PAD_X,
                pady=PAD_Y)

        # connect button
        tk.Button(center_frame, text="Connect", command=self._connect,
                  font=FONT_MEDIUM, bg=ACCENT_COLOR,
                  fg=TEXT_COLOR, relief=tk.RAISED, bd=3).grid(
            row=len(fields) + 1, column=0, columnspan=2,
            pady=PAD_Y * 3, sticky=tk.EW)

        print("[UI] Login Screen widgets created.")

    def _connect(self) -> None:
        ip: str = self.ip_var.get()

        try:
            port: int = int(self.port_var.get())
        except ValueError:
            _ = messagebox.showerror("Port", "Port must be a valid integer.")
            return
        if (port <= 1000 or port >= 65536):
            _ = messagebox.showerror("Port", "Port must be from range (1000, 65536)")
            return

        username: str = self.username_var.get()
        if ((not ip) or (not username)):
            _ = messagebox.showerror("Fields", "IP and Username cannot be empty.")
            return

        self.client.connect(ip, port, username)

class Lobby_Screen(tk.Frame):
    def __init__(self, parent: tk.Frame, ui_master: Ui, client: Client_Dummy) -> None:
        super().__init__(bg=BG_COLOR)
        self.ui: Ui = ui_master
        self.client: Client_Dummy = client

        self.room_listing_F: tk.Frame

        # main layout
        _ = self.grid_columnconfigure(0, weight=1)
        _ = self.grid_rowconfigure(1, weight=1)

        header_frame: tk.Frame = tk.Frame(self, bg=BG_COLOR)
        header_frame.grid(row=0, column=0, sticky="ew",
                          padx=PAD_X, pady=PAD_Y)

        # columns: label | spacer | buttons
        _ = header_frame.grid_columnconfigure(1, weight=1) # Spacer

        tk.Label(header_frame, text="Lobby",
                 font=FONT_LARGE, bg=BG_COLOR,
                 fg=TEXT_COLOR).grid(
                    row=0, column=0, sticky=tk.W)

        # create room button
        tk.Button(header_frame, text="Create Room",
                  command=self.ask_create_room, font=FONT_MEDIUM,
                  bg="#3498db", fg=TEXT_COLOR).grid(row=0, column=2,
                    sticky=tk.E, padx=PAD_X)

        # refresh button
        tk.Button(header_frame, text="Refresh Rooms",
                  command=self.ask_refresh_rooms, font=FONT_MEDIUM,
                  bg="#3498db", fg=TEXT_COLOR).grid(row=0, column=3,
                    sticky=tk.E, padx=PAD_X)

        # leave server button
        tk.Button(header_frame, text="Leave Server",
                  command=self.ask_disconnect, font=FONT_MEDIUM,
                  bg=ACCENT_COLOR, fg=TEXT_COLOR).grid(
                    row=0, column=4, sticky=tk.E)

        # container for the dynamic list of rooms
        self.room_list_frame: tk.Frame = tk.Frame(self, bg=BG_COLOR,
                padx=PAD_X, pady=PAD_Y)
        self.room_list_frame.grid(row=1, column=0, sticky="nsew")
        _ = self.room_list_frame.grid_columnconfigure(0, weight=1)

        # just show what is known now
        self.refresh_room_list()

    def ask_create_room(self) -> None:
        self.client.create_room()

    def ask_disconnect(self) -> None:
        self.client.disconnect()

    def ask_refresh_rooms(self) -> None:
        self.client.rooms()

    def ask_join_room(self, room_id: int) -> None:
        self.client.join(room_id)

    def get_client_rooms(self) -> list[Room]:
        return self.client.known_rooms()

    def refresh_room_list(self) -> None:
        """
        Clear All & redraw based on what is in client
        """
        # clear
        for widget in self.room_list_frame.winfo_children():
            widget.destroy()

        # header row
        header_row: tk.Frame = tk.Frame(self.room_list_frame,
                    bg=BG_COLOR, padx=PAD_X, pady=PAD_Y)
        header_row.grid(row=0, column=0, sticky="ew")
        _ = header_row.grid_columnconfigure(0, weight=1) # room id
        _ = header_row.grid_columnconfigure(1, weight=1) # state
        _ = header_row.grid_columnconfigure(2, weight=1) # join button

        tk.Label(header_row, text="Room ID", font=FONT_MEDIUM + " underline", bg=BG_COLOR, fg=TEXT_COLOR).grid(row=0, column=0, sticky=tk.W)
        tk.Label(header_row, text="State", font=FONT_MEDIUM + " underline", bg=BG_COLOR, fg=TEXT_COLOR).grid(row=0, column=1, sticky=tk.W)
        tk.Label(header_row, text="Action", font=FONT_MEDIUM + " underline", bg=BG_COLOR, fg=TEXT_COLOR).grid(row=0, column=3, sticky=tk.E)

        # Draw room rows
        for i, room in enumerate(self.get_client_rooms()):
            room_id: int = room.id
            room_state: str = room.state.lower()

            # alternate row colors
            row_color: str = "#34495e" if i % 2 == 0 else "#4e6a87"

            room_row: tk.Frame = tk.Frame(self.room_list_frame, bg=row_color,
                        padx=PAD_X, pady=PAD_Y // 2)
            room_row.grid(row=i + 1, column=0, sticky="ew")
            _ = room_row.grid_columnconfigure(0, weight=1)
            _ = room_row.grid_columnconfigure(1, weight=1)
            _ = room_row.grid_columnconfigure(2, weight=1)

            # room id
            tk.Label(room_row, text=f"Room {room_id}", font=FONT_MEDIUM,
                bg=row_color, fg=TEXT_COLOR).grid(
                row=0, column=0, sticky=tk.W)

            # state
            state_fg: str = "#2ecc71" if room_state == "open" else TEXT_COLOR
            tk.Label(room_row, text=room_state.capitalize(),
                font=FONT_MEDIUM, bg=row_color, fg=state_fg).grid(
                row=0, column=1, sticky=tk.W)

            # join button
            if room_state == "open":
                tk.Button(room_row, text="JOIN",
                    command=lambda r_id=room_id: self.ask_join_room(r_id),
                    font=FONT_SMALL + " bold", bg="#2ecc71",
                    fg=TEXT_COLOR).grid(row=0, column=2, sticky=tk.E)
            else:
                tk.Label(room_row, text="-", font=FONT_SMALL,
                    bg=row_color, fg=TEXT_COLOR).grid(
                        row=0, column=2, sticky=tk.E)

class Game_Screen(tk.Frame):
    def __init__(self, parent: tk.Frame, ui_master: Ui, client: Client_Dummy) -> None:
        super().__init__(bg=TABLE_COLOR)
        self.ui: Ui = ui_master
        self.client: Client_Dummy = client

        # container for cards
        self.player_hand_F: tk.Frame
        # label for the top card on pile
        self.pile_label: tk.Label

        # setup layout
        _ = self.grid_columnconfigure(0, weight=1)
        _ = self.grid_rowconfigure(0, weight=1) # game area
        _ = self.grid_rowconfigure(1, weight=0) # control area

        # top area (opponent & status)
        top_frame: tk.Frame = tk.Frame(self, bg=TABLE_COLOR)
        top_frame.grid(row=0, column=0, sticky="new",
                       padx=PAD_X * 2, pady=PAD_Y * 2)
        _ = top_frame.grid_columnconfigure(0, weight=1)
        _ = top_frame.grid_columnconfigure(1, weight=1)

        opponent_hand_frame: tk.Frame = tk.Frame(top_frame, bg=TABLE_COLOR)
        opponent_hand_frame.grid(row=0, column=0, sticky="nsew")
        n_cards: int = self.client.opponent_n_cards()
        self.draw_opponent_cards(opponent_hand_frame, n_cards)

        # middle area (pile & deck)
        middle_frame: tk.Frame = tk.Frame(self, bg=TABLE_COLOR)
        middle_frame.grid(row=0, column=0, sticky="nsew")
        _ = middle_frame.grid_columnconfigure(0, weight=1)
        _ = middle_frame.grid_columnconfigure(1, weight=0) # Card area
        _ = middle_frame.grid_columnconfigure(2, weight=1)

        # = deck
        deck_image: ImageTk.PhotoImage = self.ui.img_loader.get_image(CARD_BACK)
        deck_btn: tk.Button = tk.Button(middle_frame, image=deck_image,
                command=self.client.draw_card, bg=TABLE_COLOR, bd=0, relief=tk.FLAT)
        deck_btn.grid(row=0, column=0, padx=CARD_WIDTH//2, pady=CARD_HEIGHT//2,
                      sticky=tk.E)
        tk.Label(middle_frame, text="Deck (Click to Draw)", bg=TABLE_COLOR,
                 fg=TEXT_COLOR, font=FONT_SMALL).grid(
            row=1, column=0, sticky=tk.E)

        # = pile (top card)
        self.pile_label = tk.Label(middle_frame, bg=TABLE_COLOR, bd=1, relief=tk.SOLID)
        self.pile_label.grid(row=0, column=1, padx=PAD_X * 2, pady=PAD_Y * 2)
        top_card: Card = self.client.get_top_card()
        self.update_pile(top_card.__str__())

        # bottom area
        bottom_frame: tk.Frame = tk.Frame(self, bg=BG_COLOR)
        bottom_frame.grid(row=2, column=0, sticky="ew")
        _ = bottom_frame.grid_columnconfigure(0, weight=1)
        _ = bottom_frame.grid_columnconfigure(1, weight=0)

        # = player hand
        self.player_hand_F = tk.Frame(bottom_frame, bg=BG_COLOR)
        self.player_hand_F.grid(row=0, column=0, padx=PAD_X, pady=PAD_Y)

        # = controls
        control_frame: tk.Frame = tk.Frame(bottom_frame, bg=BG_COLOR)
        control_frame.grid(row=0, column=1, padx=PAD_X, pady=PAD_Y)

        tk.Button(control_frame, text="Leave Room", command=self.client.leave_room,
                 font=FONT_MEDIUM, bg=ACCENT_COLOR, fg=TEXT_COLOR).pack(pady=PAD_Y)

    def draw_opponent_cards(self, frame: tk.Frame, count: int) -> None:
        """
        Show opponents cards - only their backs
        """
        card_back_img: ImageTk.PhotoImage = self.ui.img_loader.get_image(CARD_BACK)

        overlap: float = 0.6
        x_pos: int = 0

        for i in range(count):
            label: tk.Label = tk.Label(frame, image=card_back_img,
                    bg=TABLE_COLOR, bd=0)
            label.place(x=x_pos, y=0)
            x_pos += int(CARD_WIDTH * overlap)

        # adjust frame size to fit all cards
        w: int = x_pos - int(CARD_WIDTH * overlap) + CARD_WIDTH
        _ = frame.config(width=w, height=CARD_HEIGHT)

    def update_pile(self, top_card: str) -> None:
        pile_image: ImageTk.PhotoImage = self.ui.img_loader.get_image(top_card + ".jpeg")
        _ = self.pile_label.config(image=pile_image,
                                width=CARD_WIDTH, height=CARD_HEIGHT)

    def update_hand(self, hand: list[Card]) -> None:
        # clear existing cards
        for widget in self.player_hand_F.winfo_children():
            widget.destroy()

        # center the hand
        _ = self.player_hand_F.grid_columnconfigure(0, weight=1)

        # redraw
        for i, card in enumerate(hand):
            card_image: ImageTk.PhotoImage =\
                self.ui.img_loader.get_image(card.__str__() + ".jpeg")

            # clickable card
            card_btn: tk.Button = tk.Button(self.player_hand_F,
                image=card_image, command=lambda name=card:\
                self.client.play_card(name),bg=BG_COLOR, bd=1, relief=tk.RAISED)
            card_btn.grid(row=0, column=i, padx=PAD_X//2)







