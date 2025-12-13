import socket
import threading
import sys
import time
import queue
import signal
import curses
from datetime import datetime

# --- CONFIGURATION ---
SERVER_IP = "127.0.0.1"  # Change this to your target IP
SERVER_PORT = 42690       # Change this to your target PORT
LOG_FILE = "client_traffic.log"

# --- PROTOCOL CONSTANTS ---
MAGIC = "PRSI"
DELIM = "|"
# Protocol structure requires whitespaces.
# We will use this template: " " + MAGIC + " " + CMD + " " + DELIM + " "

class AppState:
    def __init__(self):
        self.running = True
        self.pong_enabled = True
        self.socket = None
        self.ui_queue = queue.Queue()
        self.input_buffer = ""
        self.logs = []  # In-memory logs for UI
        self.max_log_lines = 100

state = AppState()

def write_to_file(direction, message):
    """
    Appends raw traffic to file.
    direction: 'SENT' or 'RECV'
    """
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    try:
        with open(LOG_FILE, "a", encoding="utf-8") as f:
            # We explicitly write the exact content, but strictly speaking
            # for a log file, we usually want a newline separator for readability
            f.write(f"[{timestamp}] [{direction}] {message}\n")
    except Exception as e:
        state.ui_queue.put(f"[SYSTEM] Error writing to file: {e}")

def format_message(command):
    """Wraps command in protocol specific whitespaces and delimiters."""
    # " " + MAGIC + " " + command + " " + DELIM + " "
    return f" {MAGIC}   {command}   {DELIM} "

def network_thread(sock):
    """
    Listens for incoming data, handles the buffer,
    auto-replies PING/PONG, and logs data.
    """
    buffer = ""

    while state.running:
        try:
            data = sock.recv(4096)
            if not data:
                state.ui_queue.put("[SYSTEM] Server closed connection.")
                state.running = False
                break

            raw_text = data.decode('utf-8', errors='ignore')
            buffer += raw_text

            # Process messages splitting by Delimiter
            while DELIM in buffer:
                # Split only on the first occurrence
                raw_msg, buffer = buffer.split(DELIM, 1)

                # The raw_msg contains MAGIC + Command.
                # We need to clean it up to find the command.
                # Expected: whitespace PRSI whitespace COMMAND whitespace

                clean_msg = raw_msg.strip()

                # Log EVERYTHING received to file
                # We reconstruct the packet logic for the file log to include the delimiter
                write_to_file("RECV", raw_msg + DELIM)

                if MAGIC in clean_msg:
                    # Remove MAGIC and find the actual command
                    # clean_msg is like "PRSI   PING"
                    parts = clean_msg.split(MAGIC)
                    if len(parts) > 1:
                        command = parts[1].strip()

                        if command == "PING":
                            if state.pong_enabled:
                                # Send PONG
                                response = format_message("PONG")
                                try:
                                    sock.sendall(response.encode('utf-8'))
                                    write_to_file("SENT", response)
                                except OSError:
                                    break
                            else:
                                # Logic requires we just don't send back,
                                # but we still logged the PING above.
                                pass
                        else:
                            # It is a normal message, show to user
                            state.ui_queue.put(f"[SERVER] {command}")
                else:
                    # Garbage data or malformed protocol
                    state.ui_queue.put(f"[RAW] {clean_msg}")

        except OSError:
            if state.running:
                state.ui_queue.put("[SYSTEM] Socket error.")
                state.running = False
            break

def draw_ui(stdscr):
    """
    Main UI loop using Curses.
    """
    # Setup Curses
    curses.curs_set(1) # Show cursor
    stdscr.nodelay(True) # Non-blocking input
    stdscr.keypad(True) # Handle special keys

    # Colors
    curses.start_color()
    curses.init_pair(1, curses.COLOR_GREEN, curses.COLOR_BLACK) # Input
    curses.init_pair(2, curses.COLOR_CYAN, curses.COLOR_BLACK)  # System
    curses.init_pair(3, curses.COLOR_YELLOW, curses.COLOR_BLACK) # Status bar
    curses.init_pair(4, curses.COLOR_RED, curses.COLOR_BLACK) # PONG OFF

    input_str = ""

    while state.running:
        height, width = stdscr.getmaxyx()

        # 1. Process Queue (New messages from network)
        try:
            while True:
                msg = state.ui_queue.get_nowait()
                state.logs.append(msg)
                if len(state.logs) > state.max_log_lines:
                    state.logs.pop(0)
        except queue.Empty:
            pass

        # 2. Draw Log Area (Top part)
        stdscr.erase()

        # Draw status bar at the top
        status_color = curses.color_pair(3)
        pong_status = "ACTIVE" if state.pong_enabled else "PAUSED (Ctrl+C to resume)"
        pong_color = curses.color_pair(1) if state.pong_enabled else curses.color_pair(4)

        stdscr.addstr(0, 0, f"Connected to {SERVER_IP}:{SERVER_PORT} | Ctrl+D to Exit | PONG Reply: ", status_color)
        stdscr.addstr(0, 56, pong_status, pong_color)
        stdscr.addstr(1, 0, "-" * (width - 1))

        # Calculate printable area
        log_height = height - 4 # Reserve line 0 (status), 1 (line), and bottom 2 lines
        start_index = max(0, len(state.logs) - log_height)

        for i, line in enumerate(state.logs[start_index:]):
            # Truncate line if too long for screen width
            display_line = line[:width-1]
            try:
                # 2 (status+line) + i
                stdscr.addstr(2 + i, 0, display_line)
            except curses.error:
                pass

        # 3. Draw Input Area (Bottom)
        input_y = height - 1
        stdscr.addstr(input_y, 0, "> " + input_str, curses.color_pair(1))

        # 4. Handle Input
        try:
            key = stdscr.getch()

            if key != -1:
                # Ctrl+D (EOT) check
                if key == 4:
                    state.running = False

                # Ctrl+C check (ASCII 3)
                elif key == 3:
                    state.pong_enabled = not state.pong_enabled
                    status_msg = "Resumed PONG responses." if state.pong_enabled else "Stopped PONG responses."
                    state.ui_queue.put(f"[SYSTEM] {status_msg}")

                # Enter Key
                elif key == 10 or key == 13:
                    if input_str.strip():
                        cmd = input_str.strip()
                        if state.socket:
                            # Format and Send
                            final_payload = format_message(cmd)
                            try:
                                state.socket.sendall(final_payload.encode('utf-8'))
                                write_to_file("SENT", final_payload)
                                state.ui_queue.put(f"[YOU] {cmd}")
                            except Exception as e:
                                state.ui_queue.put(f"[ERROR] Send failed: {e}")

                        input_str = ""

                # Backspace
                elif key == curses.KEY_BACKSPACE or key == 127 or key == 8:
                    input_str = input_str[:-1]

                # Standard characters
                elif 32 <= key <= 126:
                    input_str += chr(key)

        except KeyboardInterrupt:
            # Fallback if raw mode leaks the signal
            state.pong_enabled = not state.pong_enabled

        stdscr.refresh()
        # Sleep briefly to lower CPU usage
        time.sleep(0.05)

def main():
    # 1. Connect
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER_IP, SERVER_PORT))
        state.socket = s
    except Exception as e:
        print(f"Could not connect to {SERVER_IP}:{SERVER_PORT}")
        print(f"Error: {e}")
        return

    # 2. Start Network Thread
    t = threading.Thread(target=network_thread, args=(s,), daemon=True)
    t.start()

    # 3. Start UI Loop (Main Thread)
    try:
        # wrapper handles initialization and cleanup of curses
        curses.wrapper(draw_ui)
    except KeyboardInterrupt:
        # Catch ctrl+c if it kills wrapper before loop handles it
        pass
    finally:
        state.running = False
        s.close()
        print("Disconnected.")
        print(f"Traffic log saved to {LOG_FILE}")

if __name__ == "__main__":
    main()
