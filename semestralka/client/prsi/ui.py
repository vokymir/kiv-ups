import tkinter as tk

from prsi import config

class Client: pass # forward declare

class Ui(tk.Tk):
    def __init__(self, client: Client) -> None:
        super().__init__()
        self.client: Client = client

        self.title(config.APP_TITLE)
        self.geometry(f"{config.WINDOW_WIDTH}x{config.WINDOW_HEIGHT}")
        _ =self.configure(bg=config.BG_COLOR)

