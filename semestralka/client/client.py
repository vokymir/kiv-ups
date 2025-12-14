from __future__ import annotations
from typing import Final

import tkinter as tk
import socket

# config

SERVER: Final[str] = "127.0.0.1"
PORT: Final[int] = 3507
MAGIC: Final[str] = "PRSI"
DELIM: Final[str] = "|"

BUFFER_SIZE: Final[int] = 4096

SUITS: Final[list[str]] = ["Z", "K", "S", "L"]
RANKS: Final[list[str]] = ["7", "8", "9", "0", "J", "Q", "K", "A"]

TITLE: Final[str] = "Prší"

FONT: str = "Arial"
DEF_DIMENSIONS: Final[str] = "800x600"
CLR_BG: Final[str] = "#1a1a2e"


# client

class PrsiClient:
    def __init__(self, tktr: tk.Tk):
        self.tktr: tk.Tk = tktr
        tktr.title(TITLE)
        tktr.geometry(DEF_DIMENSIONS)
        _ = tktr.configure(bg=CLR_BG)

        self.sock: socket.socket | None = None
        self.username: str = ""
        self.room_id: int | None = None
        # TODO: default game state?

        self.frames: dict[str, tk.Frame] = {}

# main

def main() -> None:
    tktr = tk.Tk()
    _app = PrsiClient(tktr)
    tktr.mainloop()


if __name__ == "__main__":
    main()
