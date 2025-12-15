from typing import Final, TypedDict

TITLE: Final[str] = "Prší TCP Client"
WINDOW_GEOMETRY: Final[str] = "800x600"

SERVER: Final[str] = "127.0.0.1"
PORT: Final[int] = 3507


class Colors(TypedDict):
    bg_main: str
    bg_card: str
    bg_input: str
    bg_board: str
    bg_top: str
    fg_main: str
    fg_accent: str
    fg_muted: str
    danger: str


COLORS: Final[Colors] = {
    "bg_main": "#1a1a2e",
    "bg_card": "#16213e",
    "bg_input": "#0f3460",
    "bg_board": "#1b262c",
    "bg_top": "#151525",
    "fg_main": "#e0e0e0",
    "fg_accent": "#e94560",
    "fg_muted": "#888",
    "danger": "#e84118",
}

FONT_TITLE: Final[tuple[str, int, str]] = ("Arial", 24, "bold")
FONT_HEADER: Final[tuple[str, int, str]] = ("Arial", 20, "bold")
FONT_LABEL: Final[tuple[str, int]] = ("Arial", 14)
