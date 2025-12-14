import tkinter as tk
from tkinter import ttk, messagebox
from PIL import Image, ImageTk, ImageDraw
import os
from typing import Any, Dict, List, Union, Callable, Tuple
# Changed to relative import
from . import config

# Forward declaration for type hinting complex classes
class GameClient: pass
class PrsiApp: pass
class LoginFrame(tk.Frame): pass
class LobbyFrame(tk.Frame): pass
class GameFrame(tk.Frame): pass


class ResourceManager:
    """Helper to load and cache images."""
    def __init__(self) -> None:
        # Cache stores: {key: ImageTk.PhotoImage}
        self.cache: Dict[str, ImageTk.PhotoImage] = {}
        self.placeholder_color: str = "#3498db"

    def get_card_image(self, card_name: str, width: int, height: int) -> ImageTk.PhotoImage:
        """
        Loads image from assets/card_name.jpg.
        If not found, creates a generated placeholder.
        """
        key: str = f"{card_name}_{width}_{height}"
        if key in self.cache:
            return self.cache[key]

        path: str = os.path.join(config.ASSETS_DIR, f"{card_name}.jpg")

        try:
            if os.path.exists(path):
                img: Image.Image = Image.open(path)
                img = img.resize((width, height), Image.Resampling.LANCZOS)
            else:
                raise FileNotFoundError
        except Exception:
            # Create a placeholder image if file missing or corrupted
            img = Image.new('RGB', (width, height), color=self.placeholder_color)
            d = ImageDraw.Draw(img)
            # Center the placeholder text
            text_x = 10
            text_y = 10
            d.text((text_x, text_y), card_name.upper(), fill="white")
            d.rectangle([0, 0, width-1, height-1], outline="black", width=2)

        photo: ImageTk.PhotoImage = ImageTk.PhotoImage(img)
        self.cache[key] = photo
        return photo

class PrsiApp(tk.Tk):
    def __init__(self, controller: Any) -> None: # Using Any for controller to avoid circular dependency with client.py
        super().__init__()
        self.controller: Any = controller
        self.title(config.APP_TITLE)
        self.geometry(f"{config.WINDOW_WIDTH}x{config.WINDOW_HEIGHT}")
        self.configure(bg=config.BG_COLOR)

        self.resources: ResourceManager = ResourceManager()

        # Container for frames
        self.container: tk.Frame = tk.Frame(self, bg=config.BG_COLOR)
        self.container.pack(fill="both", expand=True)

        # frames stores: {page_name: tk.Frame instance}
        self.frames: Dict[str, tk.Frame] = {}
        self.current_frame: Union[str, None] = None
        self.btn_rules: tk.Button # Defined in _setup_global_ui

        # Initialize global buttons (Help, etc.)
        self._setup_global_ui()

        # Initialize View Frames
        frame_classes: Tuple[Callable[..., tk.Frame], ...] = (LoginFrame, LobbyFrame, GameFrame)
        for F in frame_classes:
            page_name: str = F.__name__
            frame: tk.Frame = F(parent=self.container, app=self)
            self.frames[page_name] = frame
            frame.grid(row=0, column=0, sticky="nsew")

        self.show_frame("LoginFrame")

    def _setup_global_ui(self) -> None:
        """Global UI elements overlaying everything."""
        self.btn_rules = tk.Button(self, text="?", font=("Arial", 14, "bold"),
                                   bg=config.ACCENT_COLOR, fg="white",
                                   command=self.show_rules)
        self.btn_rules.place(x=config.WINDOW_WIDTH - 50, y=10)

    def show_frame(self, page_name: str) -> None:
        frame: tk.Frame = self.frames[page_name]
        frame.tkraise()
        self.current_frame = page_name

        # Update buttons depending on context
        if page_name == "LoginFrame":
            self.btn_rules.place_forget() # Hide help on login
        else:
            self.btn_rules.place(x=config.WINDOW_WIDTH - 50, y=10)

    def show_rules(self) -> None:
        RulesWindow(self, self.resources)

    def get_frame(self, class_name: str) -> tk.Frame:
        return self.frames[class_name]

# --- FRAMES ---

class LoginFrame(tk.Frame):
    def __init__(self, parent: tk.Frame, app: PrsiApp) -> None:
        super().__init__(parent, bg=config.BG_COLOR)
        self.app: PrsiApp = app

        # Center Frame using place() with anchor for responsiveness
        center_frame: tk.Frame = tk.Frame(self, bg=config.BG_COLOR)
        center_frame.place(relx=0.5, rely=0.5, anchor="center")

        tk.Label(center_frame, text="PRŠÍ ONLINE", font=("Helvetica", 32, "bold"),
                 bg=config.BG_COLOR, fg=config.TEXT_COLOR).pack(pady=30)

        # Inputs
        self.ip_var: tk.StringVar = tk.StringVar(value=config.DEFAULT_IP)
        self.port_var: tk.StringVar = tk.StringVar(value=str(config.DEFAULT_PORT))
        self.user_var: tk.StringVar = tk.StringVar(value="Player1")

        self._create_entry(center_frame, "IP Address:", self.ip_var)
        self._create_entry(center_frame, "Port:", self.port_var)
        self._create_entry(center_frame, "Username:", self.user_var)

        btn: tk.Button = tk.Button(center_frame, text="CONNECT", font=("Arial", 12, "bold"),
                        bg=config.TABLE_COLOR, fg="white", width=20, height=2,
                        command=self.on_connect)
        btn.pack(pady=20)

    def _create_entry(self, parent: tk.Frame, label: str, var: tk.StringVar) -> None:
        frame: tk.Frame = tk.Frame(parent, bg=config.BG_COLOR)
        frame.pack(pady=5)
        tk.Label(frame, text=label, width=15, anchor="e", bg=config.BG_COLOR, fg=config.TEXT_COLOR).pack(side="left")
        tk.Entry(frame, textvariable=var, font=("Arial", 11)).pack(side="left", padx=5)

    def on_connect(self) -> None:
        ip: str = self.ip_var.get()
        port: str = self.port_var.get()
        user: str = self.user_var.get()
        self.app.controller.request_login(ip, port, user)


class LobbyFrame(tk.Frame):
    def __init__(self, parent: tk.Frame, app: PrsiApp) -> None:
        super().__init__(parent, bg=config.BG_COLOR)
        self.app: PrsiApp = app
        self.canvas: tk.Canvas
        self.scrollbar: ttk.Scrollbar
        self.scrollable_frame: tk.Frame

        # Header
        header: tk.Frame = tk.Frame(self, bg="#34495e", height=60)
        header.pack(fill="x")
        tk.Label(header, text="GAME LOBBY", font=("Arial", 20, "bold"), bg="#34495e", fg="white").pack(side="left", padx=20, pady=10)

        tk.Button(header, text="LEAVE SERVER", bg=config.ACCENT_COLOR, fg="white",
                  command=self.app.controller.request_leave_server).pack(side="right", padx=60, pady=10)

        # Room List (Scrollable)
        list_container: tk.Frame = tk.Frame(self, bg=config.BG_COLOR)
        list_container.pack(fill="both", expand=True, padx=50, pady=20)

        self.canvas = tk.Canvas(list_container, bg=config.BG_COLOR, highlightthickness=0)
        self.scrollbar = ttk.Scrollbar(list_container, orient="vertical", command=self.canvas.yview)
        self.scrollable_frame = tk.Frame(self.canvas, bg=config.BG_COLOR)

        self.scrollable_frame.bind(
            "<Configure>",
            lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all"))
        )
        self.canvas.create_window((0, 0), window=self.scrollable_frame, anchor="nw")
        self.canvas.configure(yscrollcommand=self.scrollbar.set)

        self.canvas.pack(side="left", fill="both", expand=True)
        self.scrollbar.pack(side="right", fill="y")

    def update_rooms(self, room_list: List[Dict[str, Any]]) -> None:
        """
        Rebuilds the room list.
        room_list expected format: [{'id': 1, 'state': 'OPEN', 'players': '1/2'}, ...]
        """
        for widget in self.scrollable_frame.winfo_children():
            widget.destroy()

        for room in room_list:
            self._create_room_row(room)

    def _create_room_row(self, room: Dict[str, Any]) -> None:
        row: tk.Frame = tk.Frame(self.scrollable_frame, bg="white", pady=10)
        row.pack(fill="x", pady=5, padx=5)

        info_text: str = f"Room #{room['id']} | State: {room['state']} | Players: {room.get('players', '?')}"
        tk.Label(row, text=info_text, font=("Arial", 12), bg="white").pack(side="left", padx=20)

        if room['state'] == "OPEN":
            btn: tk.Button = tk.Button(row, text="JOIN", bg=config.TABLE_COLOR, fg="white",
                            command=lambda r=room['id']: self.app.controller.request_join_room(r))
            btn.pack(side="right", padx=20)
        else:
            tk.Label(row, text="LOCKED", fg="gray", bg="white").pack(side="right", padx=20)


class GameFrame(tk.Frame):
    def __init__(self, parent: tk.Frame, app: PrsiApp) -> None:
        super().__init__(parent, bg=config.TABLE_COLOR)
        self.app: PrsiApp = app
        self.lbl_deck: tk.Label
        self.lbl_played: tk.Label
        self.hand_area: tk.Frame
        self.player_cards_frame: tk.Frame
        self.opponent_cards_frame: tk.Frame

        # --- Top: Opponent & Controls ---
        top_bar: tk.Frame = tk.Frame(self, bg="#1e8449", height=100)
        top_bar.pack(side="top", fill="x")

        # Leave Button
        tk.Button(top_bar, text="LEAVE ROOM", bg=config.ACCENT_COLOR, fg="white",
                  command=self.app.controller.request_leave_room).pack(side="left", padx=10, pady=10)

        # Opponent Area
        self.opponent_area: tk.Frame = tk.Frame(top_bar, bg="#1e8449")
        self.opponent_area.pack(side="top", pady=5)
        self.lbl_opponent_name: tk.Label = tk.Label(self.opponent_area, text="Waiting...", bg="#1e8449", fg="white", font=("Arial", 12, "bold"))
        self.lbl_opponent_name.pack()

        self.opponent_cards_frame = tk.Frame(self.opponent_area, bg="#1e8449")
        self.opponent_cards_frame.pack(pady=5)

        # --- Middle: Deck & Played Card ---
        middle_area: tk.Frame = tk.Frame(self, bg=config.TABLE_COLOR)
        middle_area.pack(expand=True)

        # Deck
        self.lbl_deck = tk.Label(middle_area, bg=config.TABLE_COLOR, text="DECK")
        self.lbl_deck.pack(side="left", padx=20)

        # Last Played
        self.lbl_played = tk.Label(middle_area, bg=config.TABLE_COLOR, text="No Card")
        self.lbl_played.pack(side="left", padx=20)

        # --- Bottom: Player Hand ---
        self.hand_area = tk.Frame(self, bg="#1e8449", height=200)
        self.hand_area.pack(side="bottom", fill="x", pady=20)

        tk.Label(self.hand_area, text="YOUR HAND", bg="#1e8449", fg="#bdc3c7").pack()

        self.player_cards_frame = tk.Frame(self.hand_area, bg="#1e8449")
        self.player_cards_frame.pack(pady=10)

    def set_opponent_hand(self, count: int) -> None:
        """Draws card backs for opponent."""
        for w in self.opponent_cards_frame.winfo_children():
            w.destroy()

        img: ImageTk.PhotoImage = self.app.resources.get_card_image("back", 50, 70) # Smaller for opponent

        for _ in range(count):
            lbl: tk.Label = tk.Label(self.opponent_cards_frame, image=img, bg="#1e8449")
            lbl.image = img # Keep a reference
            lbl.pack(side="left", padx=2)

    def set_table_card(self, card_name: str) -> None:
        img: ImageTk.PhotoImage = self.app.resources.get_card_image(card_name, config.CARD_WIDTH, config.CARD_HEIGHT)
        self.lbl_played.config(image=img, text="")
        self.lbl_played.image = img # Keep a reference

    def set_deck_visible(self, visible: bool = True) -> None:
        if visible:
            img: ImageTk.PhotoImage = self.app.resources.get_card_image("back", config.CARD_WIDTH, config.CARD_HEIGHT)
            self.lbl_deck.config(image=img, text="")
            self.lbl_deck.image = img # Keep a reference
            # Add click to draw
            self.lbl_deck.bind("<Button-1>", lambda e: self.app.controller.request_draw_card())
        else:
            self.lbl_deck.config(image="", text="Empty")
            self.lbl_deck.unbind("<Button-1>")

    def update_player_hand(self, card_names: List[str]) -> None:
        """
        card_names: list of strings e.g. ['ace_hearts', '7_spades']
        """
        for w in self.player_cards_frame.winfo_children():
            w.destroy()

        if len(card_names) > 9:
             messagebox.showinfo("Game Over", "You have too many cards! You lose.")

        for card in card_names:
            img: ImageTk.PhotoImage = self.app.resources.get_card_image(card, config.CARD_WIDTH, config.CARD_HEIGHT)
            btn: tk.Button = tk.Button(self.player_cards_frame, image=img, bg="#1e8449", borderwidth=0,
                            command=lambda c=card: self.app.controller.request_play_card(c))
            btn.image = img # Keep a reference
            btn.pack(side="left", padx=5)

class RulesWindow(tk.Toplevel):
    def __init__(self, parent: tk.Toplevel, resources: ResourceManager) -> None:
        super().__init__(parent)
        self.title("Prší Rules")
        self.geometry("500x600")

        canvas: tk.Canvas = tk.Canvas(self)
        scrollbar: ttk.Scrollbar = ttk.Scrollbar(self, orient="vertical", command=canvas.yview)
        scroll_frame: tk.Frame = tk.Frame(canvas)

        scroll_frame.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=scroll_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        tk.Label(scroll_frame, text="Game Rules", font=("Arial", 18, "bold")).pack(pady=10)

        for card_type, desc in config.RULES_TEXT.items():
            row: tk.Frame = tk.Frame(scroll_frame, pady=10, padx=10, relief="groove", borderwidth=1)
            row.pack(fill="x", padx=10, pady=5)

            tk.Label(row, text=card_type, font=("Arial", 12, "bold"), width=20, anchor="w").pack(side="left")

            tk.Message(row, text=desc, width=250).pack(side="left")
