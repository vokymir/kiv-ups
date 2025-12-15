# net
DEFAULT_IP = "127.0.0.1"
DEFAULT_PORT = 3507

BUF_SIZE = 4096

# ui
WINDOW_WIDTH = 1024
WINDOW_HEIGHT = 768
APP_TITLE = "Prší"

BG_COLOR = "#2C3E50"        # dark blue/grey
TABLE_COLOR = "#27ae60"     # forest green (card table)
ACCENT_COLOR = "#e74c3c"    # red
TEXT_COLOR = "#ecf0f1"      # white
CARD_BG = "#ffffff"

CARD_WIDTH = 100
CARD_HEIGHT = 145
ASSETS_DIR = "assets"       # directory where .jpg images are stored

# protocol
PROTO_MAGIC = "PRSI"
PROTO_DELIM = "|"

CMD_NAME = "NAME"

CMD_ROOMS = "LIST_ROOMS"
CMD_JOIN = "JOIN_ROOM"
CMD_LEAVE_ROOM = "LEAVE_ROOM"

CMD_LEAVE_SERVER = "LEAVE_SERVER" # everything invalid would work

# rules - we will see about that
RULES_TEXT = {
    "7": "Seventh: Next player must draw 2 cards or play another 7.",
    "Ace": "Ace: You can skip the next player.",
    "Svršek (Queen/Over)": "Change Suit: You can change the current suit.",
    "Green King (King of Spades)": "Effect: Next player draws 5 cards!",
    "Basic": "Must follow suit or rank."
}
