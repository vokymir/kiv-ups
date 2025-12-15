import tkinter as tk
from tkinter import messagebox
from tracemalloc import Frame

from prsi import config

class Client: pass # forward declare

class Ui(tk.Tk):
    def __init__(self, client: Client) -> None:
        super().__init__()
        self.client: Client = client
        print("[UI] Initializing Tkinter root window (Ui)...")

        self.title(config.APP_TITLE)
        self.geometry(f"{config.WINDOW_WIDTH}x{config.WINDOW_HEIGHT}")
        _ =self.configure(bg=config.BG_COLOR)
        self.minsize(config.WINDOW_WIDTH, config.WINDOW_HEIGHT)

        # root container
        _ = self.grid_columnconfigure(0, weight=1)
        _ = self.grid_rowconfigure(0, weight=1)

        # container = main frame for screens, fills the root container
        container: tk.Frame = tk.Frame(self, bg=config.BG_COLOR)
        container.grid(row=0, column=0, sticky="nsew")

        # holder for all frames
        self.frames: dict[str, tk.Frame] = {}
        self.frames["login"] = Login_Screen(container, self, client)

        for frame in self.frames.values():
            frame.grid(row=0, column=0, sticky="nsew")

        self.switch_frame("login")
        print("[UI] Initialization complete. Login screen is set to be raised.")

    def switch_frame(self, frame_name: str) -> None:
        """
        Raise the selected frame to the top.
        """
        frame: tk.Frame = self.frames[frame_name]
        frame.tkraise()

class Login_Screen(tk.Frame):
    def __init__(self, parent: tk.Frame, ui_master: Ui, client: Client) -> None:
        super().__init__(parent)
        self.ui: Ui = ui_master
        self.client: Client = client
        print("[UI] Initializing Login Screen widgets...")

        # input variables
        self.ip: tk.StringVar = tk.StringVar(value=config.DEFAULT_IP)
        self.port: tk.StringVar = tk.StringVar(value=str(config.DEFAULT_PORT))
        self.username: tk.StringVar = tk.StringVar(value="Player")

        # positioning
        center_frame: tk.Frame = tk.Frame(self, bg=config.BG_COLOR,
                                padx=config.PAD_X * 3, pady=config.PAD_Y * 3,
                                bd=2, relief=tk.RIDGE)
        center_frame.place(relx=0.5, rely=0.5, anchor=tk.CENTER)

        # widget setup
        tk.Label(center_frame, text=config.APP_TITLE, font=config.FONT_LARGE,
                bg=config.BG_COLOR, fg=config.TEXT_COLOR).grid(row=0,
                    column=0, columnspan=2, pady=config.PAD_Y * 2)
        fields: list[tuple[str, tk.StringVar]] = [
            ("Server IP:", self.ip),
            ("Port:", self.port),
            ("Username:", self.username)
        ]

        # input box setup
        for i, (label_text, var) in enumerate(fields):
            tk.Label(center_frame, text=label_text, font=config.FONT_MEDIUM,
                     bg=config.BG_COLOR, fg=config.TEXT_COLOR).grid(
                row=i + 1, column=1, sticky=tk.E, padx=config.PAD_X,
                pady=config.PAD_Y)
            tk.Entry(center_frame, textvariable=var, font=config.FONT_MEDIUM,
                     width=25, bg=config.CARD_BG, fg="#000000").grid(
                row=i + 1, column=1, sticky=tk.E, padx=config.PAD_X,
                pady=config.PAD_Y)

        # connect button
        tk.Button(center_frame, text="Connect", command=self._connect,
                  font=config.FONT_MEDIUM, bg=config.ACCENT_COLOR,
                  fg=config.TEXT_COLOR, relief=tk.RAISED, bd=3).grid(
            row=len(fields) + 1, column=0, columnspan=2,
            pady=config.PAD_Y * 3, sticky=tk.EW)

        print("[UI] Login Screen widgets created.")

    def _connect(self) -> None:
        ip: str = self.ip.get()

        try:
            port: int = int(self.port.get())
        except ValueError:
            _ = messagebox.showerror("Port", "Port must be a valid integer.")
            return
        if (port <= 1000 or port >= 65536):
            _ = messagebox.showerror("Port", "Port must be from range (1000, 65536)")
            return

        username: str = self.username.get()
        if ((not ip) or (not username)):
            _ = messagebox.showerror("Fields", "IP and Username cannot be empty.")
            return

        self.client.connect(ip, port, username)










