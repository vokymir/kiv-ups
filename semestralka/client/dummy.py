import socket
import time
import threading
import re

# --- Protocol Configuration ---
MAGIC = "PRSI"

# The literal character used to separate messages in the stream
DELIMITER = "|"
PING_CMD = "PING"
PONG_CMD = "PONG"
NAME_CMD = "NAME"

# --- Connection Configuration ---
SERVER_IP = "127.0.0.1"  # Change this to your server's IP
SERVER_PORT = 42690      # New specified server PORT
BUFFER_SIZE = 4096

# --- Protocol Helper Functions ---

def create_message(command, *args):
    """
    Constructs a protocol message: < MAGIC COMMAND arg1 arg2 ... | >
    Ensures whitespace is around every element, ending with the bare DELIMITER character + space.
    """
    parts = [MAGIC, command] + list(args)
    # 1. Join parts with a single space.
    # 2. Add a leading space.
    # 3. Add the DELIMITER character.
    # 4. Add a trailing space (to separate from the next message).
    # Example: " PRSI PONG | "
    return " " + " ".join(parts) + " " + DELIMITER + " "

def parse_message(raw_data):
    """
    Parses a raw message string into (MAGIC, COMMAND, [ARGS]).
    The input `raw_data` is a SINGLE, fully framed message (e.g., ' PRSI PING | ').
    """
    try:
        # 1. Strip the surrounding whitespace and the defined DELIMITER
        # The message we received is ' PRSI PING | '
        # .strip() -> 'PRSI PING |'
        # .rstrip(DELIMITER) -> 'PRSI PING '
        # .strip() -> 'PRSI PING'

        # We must use raw_data.strip() to handle the leading/trailing spaces correctly,
        # and then remove the bare DELIMITER from the end.
        data = raw_data.strip().rstrip(DELIMITER).strip()

        # 2. Split by any number of contiguous whitespaces (using split() without args)
        parts = data.split()

        # 3. Basic validation
        if not parts or parts[0] != MAGIC:
            return None, None, []

        # 4. Extract MAGIC, COMMAND, and ARGS
        magic = parts[0]
        command = parts[1] if len(parts) > 1 else ""
        args = parts[2:]

        return magic, command, args
    except Exception as e:
        print(f"Error parsing message: {e}")
        return None, None, []

# --- Socket Client Implementation ---

class SimpleClient:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.socket = None
        self.running = False
        self.receive_buffer = "" # Intelligent buffer for handling partial messages

    def connect(self):
        """Initializes the socket connection."""
        try:
            print(f"Attempting to connect to {self.ip}:{self.port}...")
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.ip, self.port))
            self.running = True
            print("Connection successful.")
            return True
        except socket.error as e:
            print(f"Connection failed: {e}")
            self.running = False
            return False

    def send_message(self, raw_message):
        """Sends a raw message and prints it to the console."""
        if self.socket and self.running:
            try:
                self.socket.sendall(raw_message.encode('utf-8'))
                # The message print is stripped to look clean in the console
                print(f"[SENT]   {raw_message.strip()}")
            except socket.error as e:
                print(f"Error sending data: {e}")
                self.running = False

    def handle_incoming_data(self, data):
        """
        Processes received data using the internal buffer to handle message framing.
        Uses the bare DELIMITER character for stream breaking.
        """
        # 1. Append new data to the buffer
        self.receive_buffer += data.decode('utf-8')

        # 2. Process complete messages from the buffer
        # We look for the simple DELIMITER character (|)
        while DELIMITER in self.receive_buffer:
            # Find the end of the first complete message (the | character)
            message_end_index = self.receive_buffer.find(DELIMITER)

            # Extract the complete message (from start up to and including the |)
            full_message = self.receive_buffer[:message_end_index + len(DELIMITER)]

            # Update the buffer to contain only the remaining data
            # NOTE: We must also account for any trailing whitespace that was sent after the delimiter,
            # but for safety, we extract up to the delimiter here, and the buffer will start
            # with the next character, which is likely a space (if the server follows the rule).
            self.receive_buffer = self.receive_buffer[message_end_index + len(DELIMITER):]

            # Print the received message and handle the protocol logic
            print(f"[RECEIVED] {full_message.strip()}")
            self._process_protocol_message(full_message)

    def _process_protocol_message(self, full_message):
        """
        Handles the logic for known protocol messages like PING.
        """
        # Parse the message into its components
        magic, command, args = parse_message(full_message)

        if command == PING_CMD:
            # Respond to a PING with a PONG using the new create_message logic
            pong_msg = create_message(PONG_CMD)
            self.send_message(pong_msg)
        elif command == PONG_CMD:
            print(f"--> PONG received.")
            pass
        elif command == NAME_CMD:
            print(f"--> Server acknowledged NAME command for user: {args[0] if args else 'N/A'}")

    def receive_loop(self):
        """The main thread for continuously receiving data."""
        while self.running:
            try:
                self.socket.settimeout(0.5)
                data = self.socket.recv(BUFFER_SIZE)
                if not data:
                    print("Server closed the connection.")
                    break
                self.handle_incoming_data(data)
            except socket.timeout:
                pass
            except socket.error as e:
                if self.running:
                    print(f"Socket error: {e}")
                break
            except Exception as e:
                print(f"An unexpected error occurred: {e}")
                break
        self.running = False
        print("Receive loop terminated.")

    def run(self):
        """Starts the client, including the receiver thread and initial sends."""
        if not self.connect():
            return

        self.receiver_thread = threading.Thread(target=self.receive_loop)
        self.receiver_thread.daemon = True
        self.receiver_thread.start()

        print("\n--- Initial Protocol Flow Started ---")

        try:
            print("Waiting for 3 seconds before sending NAME command...")
            time.sleep(3)

            name_msg = create_message(NAME_CMD, "MyPythonClient")
            self.send_message(name_msg)

            time.sleep(1)

            list_msg = create_message("LIST_ROOMS")
            self.send_message(list_msg)

            while self.running:
                time.sleep(1)

        except KeyboardInterrupt:
            print("\nClient requested to shut down.")
        finally:
            self.close()

    def close(self):
        """Cleanly closes the connection."""
        self.running = False
        if self.socket:
            print("Closing socket...")
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            self.socket.close()

# --- Main Execution ---

if __name__ == '__main__':
    client = SimpleClient(SERVER_IP, SERVER_PORT)
    client.run()
