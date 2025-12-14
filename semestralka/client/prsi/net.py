import socket
import threading
import queue
import sys

class NetworkManager:
    """
    Handles socket connections in a separate thread.
    """
    def __init__(self, message_queue):
        self.socket = None
        self.is_connected = False
        self.running = False
        self.receive_thread = None

        # Queue to send data back to the Main UI thread
        self.message_queue = message_queue

    def connect(self, ip, port, username):
        """
        Connects to the C++ server.
        Returns True if successful, False otherwise.
        """
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5) # 5 second timeout for connection attempt
            self.socket.connect((ip, int(port)))
            self.socket.settimeout(None) # Remove timeout for blocking operations

            self.is_connected = True
            self.running = True

            # Start the listener thread
            self.receive_thread = threading.Thread(target=self._listen_loop, daemon=True)
            self.receive_thread.start()

            print(f"[NET] Connected to {ip}:{port} as {username}")
            return True

        except Exception as e:
            print(f"[NET] Connection error: {e}")
            self.is_connected = False
            return False

    def disconnect(self):
        """Closes the connection safely."""
        self.running = False
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
        self.is_connected = False
        print("[NET] Disconnected")

    def send_command(self, message):
        """
        Sends a string message to the server.
        Adjust encoding if your C++ server expects something else.
        """
        if not self.is_connected or not self.socket:
            print("[NET] Error: Not connected")
            return

        try:
            # Assuming line-based protocol ending with newline
            data = (message + "\n").encode('utf-8')
            self.socket.sendall(data)
            print(f"[NET] Sent: {message}")
        except Exception as e:
            print(f"[NET] Send error: {e}")
            self.disconnect()
            # Notify UI of disconnection
            self.message_queue.put(("ERROR", "Connection lost"))

    def _listen_loop(self):
        """
        Runs in a background thread. Reads from socket and puts
        messages into the queue for the UI thread to handle.
        """
        buffer = ""

        while self.running and self.socket:
            try:
                # Receive data
                data = self.socket.recv(4096)
                if not data:
                    break # Connection closed by server

                text_chunk = data.decode('utf-8')
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
