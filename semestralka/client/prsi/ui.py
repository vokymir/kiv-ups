import tkinter as tk
from tkinter import messagebox

# hehe
from prsi.config import ACCENT_COLOR, APP_TITLE, BG_COLOR, CARD_BG, DEFAULT_IP, DEFAULT_PORT, FN_LOGIN, FN_LOBBY, FN_ROOM, FONT_LARGE, FONT_MEDIUM, FONT_SMALL, PAD_X, PAD_Y, TABLE_COLOR, TEXT_COLOR, WINDOW_HEIGHT, WINDOW_WIDTH
from prsi.common import Client_Dummy, Room

class Ui(tk.Tk):
    def __init__(self, client: Client_Dummy) -> None:
        super().__init__()
        self.client: Client_Dummy = client
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

        self.frames[FN_LOGIN] = self.login_frame
        self.frames[FN_LOBBY] = self.lobby_frame

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

        # columns: label | spacer | refresh button | leave server button
        _ = header_frame.grid_columnconfigure(1, weight=1) # Spacer column

        tk.Label(header_frame, text="Lobby: Available Rooms",
                 font=FONT_LARGE, bg=BG_COLOR,
                 fg=TEXT_COLOR).grid(
                    row=0, column=0, sticky=tk.W)

        # refresh button
        tk.Button(header_frame, text="Refresh Rooms",
                  command=self.ask_refresh_rooms, font=FONT_MEDIUM,
                  bg="#3498db", fg=TEXT_COLOR).grid(row=0, column=2,
                    sticky=tk.E, padx=PAD_X)

        # leave server button
        tk.Button(header_frame, text="Leave Server",
                  command=self.ask_disconnect, font=FONT_MEDIUM,
                  bg=ACCENT_COLOR, fg=TEXT_COLOR).grid(
                    row=0, column=3, sticky=tk.E)

        # container for the dynamic list of rooms
        self.room_list_frame: tk.Frame = tk.Frame(self, bg=BG_COLOR,
                padx=PAD_X, pady=PAD_Y)
        self.room_list_frame.grid(row=1, column=0, sticky="nsew")
        _ = self.room_list_frame.grid_columnconfigure(0, weight=1)

        # just show what is known now
        self.refresh_room_list()

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
        pass








