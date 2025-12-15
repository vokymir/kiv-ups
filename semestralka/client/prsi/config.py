# net
DEFAULT_IP = "127.0.0.1"
DEFAULT_PORT = 3507

BUF_SIZE = 4096

# ui
FN_LOGIN: str = "login" # frame name
FN_LOBBY: str = "lobby"
FN_ROOM: str = "room"

WINDOW_WIDTH: int = 1024
WINDOW_HEIGHT: int = 768
APP_TITLE: str = "Prší"

BG_COLOR: str = "#2C3E50"      # dark blue/grey, background of the window
TABLE_COLOR: str = "#27ae60"   # forest green, game table background
ACCENT_COLOR: str = "#e74c3c"  # red, for buttons/highlights
TEXT_COLOR: str = "#ecf0f1"    # white/light grey, for text
CARD_BG: str = "#ffffff"       # white, internal card background

CARD_WIDTH: int = 100
CARD_HEIGHT: int = 145

PAD_X: int = 10
PAD_Y: int = 10
FONT_LARGE: str = "Helvetica 16 bold"
FONT_MEDIUM: str = "Helvetica 12"
FONT_SMALL: str = "Helvetica 10"

ASSETS_DIR: str = "assets"     # directory where .jpg images are stored

MAX_HAND_CARDS: int = 10
CARD_BACK: str = "back.jpg"

# protocol
PROTO_MAGIC = "PRSI"
PROTO_DELIM = "|"

# == control
CMD_PONG = "PONG"
CMD_LEAVE_SERVER = "LEAVE_SERVER" # everything invalid would work

# == unnamed
CMD_NAME = "NAME"

# == lobby
CMD_ROOMS = "LIST_ROOMS"

# == room
CMD_JOIN = "JOIN_ROOM"
CMD_LEAVE_ROOM = "LEAVE_ROOM"

# == game


# rules - we will see about that
RULES_TEXT = {
    "7": "Seventh: Next player must draw 2 cards or play another 7.",
    "Ace": "Ace: You can skip the next player.",
    "Svršek (Queen/Over)": "Change Suit: You can change the current suit.",
    "Green King (King of Spades)": "Effect: Next player draws 5 cards!",
    "Basic": "Must follow suit or rank."
}
