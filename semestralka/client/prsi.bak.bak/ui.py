from __future__ import annotations

import tkinter as tk
from typing import TYPE_CHECKING

from .config import COLORS, FONT_TITLE, FONT_HEADER, FONT_LABEL, SERVER, PORT

if TYPE_CHECKING:
    from .client import PrsiClient

def setup_login_frame(self: "PrsiClient") -> None:
    frame = tk.Frame(self.root, bg=COLORS["bg_main"], padx=50, pady=50)
    self.frames["login"] = frame

    frame.grid_rowconfigure((0, 2), weight=1)
    frame.grid_columnconfigure(0, weight=1)

    card = tk.Frame(
        frame,
        bg=COLORS["bg_card"],
        padx=30,
        pady=30,
        bd=5,
        relief=tk.RIDGE,
    )
    card.grid(row=1, column=0)

    tk.Label(
        card,
        text="PRŠÍ ONLINE",
        font=FONT_TITLE,
        fg=COLORS["fg_accent"],
        bg=COLORS["bg_card"],
    ).pack(pady=10)

    self.ip_entry = _labeled_entry(card, "Server IP:", SERVER)
    self.port_entry = _labeled_entry(card, "Port:", str(PORT))
    self.username_entry = _labeled_entry(card, "Username:", "Player")

    tk.Button(
        card,
        text="Connect",
        command=self.connect,
        bg=COLORS["fg_accent"],
        fg="white",
        relief=tk.FLAT,
    ).pack(pady=20, fill="x")

def setup_lobby_frame(self: "PrsiClient") -> None:
    frame = tk.Frame(self.root, bg=COLORS["bg_main"], padx=20, pady=20)
    self.frames["lobby"] = frame

    frame.grid_columnconfigure(0, weight=1)
    frame.grid_rowconfigure(1, weight=1)

    header = tk.Frame(frame, bg=COLORS["bg_main"])
    header.grid(row=0, column=0, sticky="ew", pady=(0, 10))

    tk.Label(
        header,
        text="Game Lobby",
        font=FONT_HEADER,
        fg=COLORS["fg_main"],
        bg=COLORS["bg_main"],
    ).pack(side=tk.LEFT)

    self.user_label = tk.Label(
        header,
        text="Logged in as: N/A",
        fg=COLORS["fg_muted"],
        bg=COLORS["bg_main"],
    )
    self.user_label.pack(side=tk.LEFT, padx=15)

    tk.Button(
        header,
        text="Logout",
        command=self.disconnect,
        bg=COLORS["danger"],
        fg="white",
        relief=tk.FLAT,
    ).pack(side=tk.RIGHT)

def setup_game_frame(self: "PrsiClient") -> None:
    frame = tk.Frame(self.root, bg=COLORS["bg_board"])
    self.frames["game"] = frame

    frame.grid_rowconfigure(1, weight=1)
    frame.grid_columnconfigure(0, weight=1)

    top = tk.Frame(frame, bg=COLORS["bg_top"], height=80)
    top.grid(row=0, column=0, sticky="ew")

    self.opp_name_label = tk.Label(
        top,
        text="Opponent: Waiting...",
        font=FONT_LABEL,
        fg=COLORS["fg_main"],
        bg=COLORS["bg_top"],
    )
    self.opp_name_label.pack(side=tk.LEFT, padx=20)

    self.opp_count_label = tk.Label(
        top,
        text="0",
        font=("Arial", 16, "bold"),
        fg=COLORS["fg_accent"],
        bg=COLORS["bg_top"],
    )
    self.opp_count_label.pack(side=tk.LEFT)

    tk.Button(
        top,
        text="Leave Room",
        command=self.leave_room,
        bg=COLORS["danger"],
        fg="white",
        relief=tk.FLAT,
    ).pack(side=tk.RIGHT, padx=20)

def _labeled_entry(
    parent: tk.Widget,
    label: str,
    default: str,
) -> tk.Entry:
    tk.Label(
        parent,
        text=label,
        fg=COLORS["fg_main"],
        bg=COLORS["bg_card"],
        anchor="w",
    ).pack(fill="x", pady=(10, 0))

    entry = tk.Entry(
        parent,
        width=30,
        bd=0,
        highlightthickness=0,
        bg=COLORS["bg_input"],
        fg="white",
        insertbackground="white",
    )
    entry.insert(0, default)
    entry.pack(pady=5)
    return entry
