# --- Network Defaults ---
DEFAULT_IP = "127.0.0.1"
DEFAULT_PORT = 12345

# --- UI Constants ---
WINDOW_WIDTH = 1024
WINDOW_HEIGHT = 768
APP_TITLE = "Prší Client"

# --- Colors ---
BG_COLOR = "#2C3E50"        # Dark Blue/Grey
TABLE_COLOR = "#27ae60"     # Forest Green (Card table)
ACCENT_COLOR = "#e74c3c"    # Red
TEXT_COLOR = "#ecf0f1"      # White
CARD_BG = "#ffffff"

# --- Card Settings ---
CARD_WIDTH = 100
CARD_HEIGHT = 145
ASSETS_DIR = "assets"       # Folder where .jpg images are stored

# --- Protocol / Game Constants ---
# You can define your protocol constants here
CMD_LOGIN = "LOGIN"
CMD_JOIN = "JOIN"
CMD_LEAVE_SERVER = "LEAVE_SRV"
CMD_LEAVE_ROOM = "LEAVE_ROOM"

# Placeholder rules text
RULES_TEXT = {
    "7": "Seventh: Next player must draw 2 cards or play another 7.",
    "Ace": "Ace: You can skip the next player.",
    "Svršek (Queen/Over)": "Change Suit: You can change the current suit.",
    "Green King (King of Spades)": "Effect: Next player draws 5 cards!",
    "Basic": "Must follow suit or rank."
}
