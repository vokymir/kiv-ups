import socket
import threading
import queue
from typing import Any, Tuple, Optional
# Changed to relative import
from . import config

# Define the type for messages passed through the queue: (message_type, content)
# Content can be a string or None, depending on the message type.
QueueMessage = Tuple[str, Optional[str]]

class NetworkManager:
    """
    Handles socket connections in a separate thread.
    """
    def __init__(self, message_queue: queue.Queue[QueueMessage]) -> None:
        self.socket: Optional[socket.socket] = None
        self.is_connected: bool = False
        self.running: bool = False
        self.receive_thread: Optional[threading.Thread] = None

        # Queue to send data back to the Main UI thread
        self.message_queue: queue.Queue[QueueMessage] = message_queue

    def connect(self, ip: str, port: str, username: str) -> bool:
        """
        Connects to the C++ server.
        Returns True if successful, False otherwise.
        """
        try:
            target_port: int = int(port)
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5) # 5 second timeout for connection attempt
            self.socket.connect((ip, target_port))
            self.socket.settimeout(None) # Remove timeout for blocking operations

            self.is_connected = True
            self.running = True

            # Start the listener thread
            self.receive_thread = threading.Thread(target=self._listen_loop, daemon=True)
            self.receive_thread.start()

            print(f"[NET] Connected to {ip}:{target_port} as {username}")
            return True

        except ValueError:
            print("[NET] Connection error: Port must be a valid number.")
            self.is_connected = False
            return False
        except Exception as e:
            print(f"[NET] Connection error: {e}")
            self.is_connected = False
            return False

    def disconnect(self) -> None:
        """Closes the connection safely."""
        self.running = False
        if self.socket:
            try:
                self.socket.close()
            except Exception:
                pass
        self.is_connected = False
        print("[NET] Disconnected")

    def send_command(self, message: str) -> None:
        """
        Sends a string message to the server.
        """
        if not self.is_connected or not self.socket:
            print("[NET] Error: Not connected")
            return

        try:
            # Assuming line-based protocol ending with newline
            data: bytes = (message + "\n").encode('utf-8')
            self.socket.sendall(data)
            print(f"[NET] Sent: {message}")
        except Exception as e:
            print(f"[NET] Send error: {e}")
            self.disconnect()
            # Notify UI of disconnection
            self.message_queue.put(("ERROR", "Connection lost"))

    def _listen_loop(self) -> None:
        """
        Runs in a background thread. Reads from socket and puts
        messages into the queue for the UI thread to handle.
        """
        buffer: str = ""

        while self.running and self.socket:
            try:
                # Receive data
                data: bytes = self.socket.recv(4096)
                if not data:
                    break # Connection closed by server

                text_chunk: str = data.decode('utf-8')
                buffer += text_chunk

                # Split by newline (assuming line-based protocol)
                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)
                    if line.strip():
                        # PUT MESSAGE IN QUEUE FOR MAIN THREAD
                        self.message_queue.put(("SERVER_MSG", line.strip()))

            except socket.error as e:
                print(f"[NET] Socket error: {e}")
                break
            except Exception as e:
                print(f"[NET] Error: {e}")
                break

        self.disconnect()
        self.message_queue.put(("DISCONNECTED", None))
