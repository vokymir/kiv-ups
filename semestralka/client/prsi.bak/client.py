from __future__ import annotations

import socket
import tkinter as tk

from .config import TITLE, WINDOW_GEOMETRY
from . import ui


class PrsiClient:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        root.title(TITLE)
        root.geometry(WINDOW_GEOMETRY)

        self.sock: socket.socket | None = None
        self.username: str = ""
        self.room_id: int | None = None

        self.frames: dict[str, tk.Frame] = {}

        ui.setup_login_frame(self)
        ui.setup_lobby_frame(self)
        ui.setup_game_frame(self)

        self.show_frame("login")

    def show_frame(self, name: str) -> None:
        for frame in self.frames.values():
            frame.grid_forget()
        self.frames[name].grid(row=0, column=0, sticky="nsew")

    # stubs
    def connect(self) -> None: ...
    def disconnect(self) -> None: ...
    def leave_room(self) -> None: ...
