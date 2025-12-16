import queue
import socket
import threading

from prsi.config import BUF_SIZE, PROTO_DELIM, PROTO_MAGIC

Queue_Message = tuple[str, str|None]
QM_ERROR: str = "ERROR"
QM_MESSAGE: str = "MESSAGE"
QM_DISCONNECTED: str = "DISCONNECTED"

class Net:
    def __init__(self, message_queue: queue.Queue[Queue_Message]) -> None:
        self.sock: socket.socket | None = None
        # let others know this
        self.connected: bool = False

        # shared variable to stop listen-thread
        self.running: bool = False
        self.thread: threading.Thread | None = None

        self.mq: queue.Queue[Queue_Message] = message_queue

    def connect(self, ip: str, port: str) -> bool:
        """
        Connect to specified server.
        Return true on success, false on fail.
        """
        try:
            target_port: int = int(port)
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5) # only try connecting for so long
            self.sock.connect((ip, target_port))
            self.sock.settimeout(None) # separate thread, so no timeout

            self.connected = True

            self.running = True
            self.thread = threading.Thread(target=self._listen_loop, daemon=True)
            self.thread.start()

            print(f"[NET] Connected to {ip}:{port}")
            return True

        except ValueError:
            print(f"[NET] Connection error: Port must be a valid number.")
            self.connected = False
            return False
        except Exception as e:
            print(f"[NET] Connection error: {e}")
            self.connected = False
            return False

    def disconnect(self) -> None:
        """
        Close the current connection safely.
        """
        self.running = False
        if (self.sock):
            try:
                self.sock.close()
            except Exception:
                # only not to propagate exception higher
                # we know that socket is closed anyway
                pass
        self.connected = False
        print("[NET] Disconnected")

    def send_command(self, message: str) -> None:
        """
        Send a string message to the server.
        Responsible for adding magic & delim.
        """
        if ((not self.connected) or (not self.sock)):
            print("[NET] Error: Not connected")
            return

        try:
            protocol_msg: str = " " + PROTO_MAGIC + " " + message + " " + PROTO_DELIM + " "
            data: bytes = (protocol_msg).encode('utf-8')
            self.sock.sendall(data) # TODO: maybe do periodic send?
            print(f"[NET] Sent: {message}")

        except Exception as e:
            print(f"[NET] Send error: {e}")
            self.disconnect()
            # notify UI
            self.mq.put((QM_ERROR, "Connection lost"))

    def _listen_loop(self) -> None:
        """
        Runs in bg thread. Read from socket and put whole messages into queue.
        """
        buffer: str = ""

        while (self.running and self.sock):
            try:
                # recv
                data: bytes = self.sock.recv(BUF_SIZE)
                if (not data):
                    break # connection closed by server

                text: str = data.decode('utf-8')
                buffer += text

                should_break: bool = False
                while (True):
                    buffer = buffer.lstrip()

                    # not one whole message yet
                    if (len(buffer) < len(PROTO_MAGIC)):
                        break

                    # check for magic
                    if (not buffer.startswith(PROTO_MAGIC)):
                        print("[PROTO] Received message doesn't start with magic.")
                        should_break = True
                        break

                    # check for delim = at least one message
                    if (PROTO_DELIM in buffer):
                        # find the message
                        cmd: str
                        cmd, buffer = buffer.split(PROTO_DELIM, 1)
                        if (not cmd): # check if splitted correctly
                            should_break = True
                            break

                        cmd = cmd[len(PROTO_MAGIC):] # remove magic
                        cmd = cmd.strip() # remove whitespaces

                        # let main thread know
                        self.mq.put((QM_MESSAGE, cmd))
                    else:
                        break

                if (should_break):
                    break

            except socket.error as e:
                print(f"[NET] Socket error: {e}")
                break
            except Exception as e:
                print(f"[NET] Error: {e}")
                break

        self.disconnect()
        self.mq.put((QM_DISCONNECTED, ""))

