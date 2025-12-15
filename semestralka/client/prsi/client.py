from queue import Queue
from tkinter import messagebox
import queue
from typing import override
from prsi.net import QM_DISCONNECTED, QM_ERROR, QM_MESSAGE, Net, Queue_Message
from prsi.ui import Ui
from prsi.config import CMD_DRAW, CMD_JOIN, CMD_NAME, CMD_PONG, CMD_ROOMS, FN_LOBBY, FN_LOGIN
from prsi.common import Client_Dummy, Room

class Client(Client_Dummy):
    """
    The controller, have net & ui and actually do all the stuff.
    """
    def __init__(self) -> None:
        # message queue: for getting messages out of net
        self.mq: Queue[Queue_Message] = queue.Queue()

        # stuff
        self.known_rooms_: list[Room] = []
        self.should_play_: bool = False

        # Already Sent DRAW
        self.as_draw: bool = False

        # network part of client - talk via queue
        self.net: Net = Net(self.mq)

        # ui
        self.ui: Ui = Ui(self)

        _ = self.ui.after(100, self.process_incoming_messages)
        print("[Client] Initialization complete. Ready to run.")

    @override
    def run(self) -> None:
        print("--- Starting Tkinter main loop (GUI should now appear) ---")
        self.ui.update()
        self.ui.mainloop()
        print("--- Tkinter main loop exited ---")

    # get/set

    @override
    def known_rooms(self) -> list[Room]:
        return self.known_rooms_

    # ui -> net

    @override
    def connect(self, ip: str, port: int, username: str) -> None:
        if not(self.net.connect(ip, str(port))):
            self.ui.show_temp_message("Cannot connect to the server")
            return

        self.net.send_command(CMD_NAME + " " + username)
        self.ui.show_temp_message("Trying to connect.", 1000)

    @override
    def rooms(self) -> None:
        """
        Ask server for rooms
        """
        if ()
        self.net.send_command(CMD_ROOMS)

    @override
    def join(self, room_id: int) -> None:
        self.net.send_command(CMD_JOIN + " " + str(room_id))

    @override
    def disconnect(self) -> None:
        self.net.disconnect()
        self.ui.switch_frame(FN_LOGIN)

    @override
    def draw_card(self) -> None:
        if (not self.should_play_):
            self.ui.show_temp_message("It's not your turn.")
            return
        self.net.send_command(CMD_DRAW)

    # net -> ui

    def process_incoming_messages(self) -> None:
        """
        Periodically check the queue for messages from the net.
        This method runs on the main thread = safe to update tkinter.
        """
        while (not self.mq.empty()):
            qm_type, content = self.mq.get()

            if (qm_type == QM_ERROR):
                _ = messagebox.showerror("Network Error", content if content else "Unknown Error")
                self.ui.switch_frame(FN_LOGIN)

            elif (qm_type == QM_DISCONNECTED):
                self.ui.switch_frame(FN_LOGIN)

            elif (qm_type == QM_MESSAGE):
                if (content):
                    self.handle_protocol(content)

        _ = self.ui.after(100, self.process_incoming_messages)

    def handle_protocol(self, msg: str) -> None:
        """
        Authoritative place to handle all incoming messages from net.
        """
        print(f"[PROTO] Received: {msg}")
        parts: list[str] = msg.split()
        if (not parts):
            return

        cmd: str = parts[0]

        match cmd:
            case "OK":
                if (len(parts) > 1):
                    if (parts[1] == "NAME"):
                        self.net.send_command(CMD_ROOMS)

                return
            case "ROOMS":
                self.parse_rooms_message(parts)
                self.ui.refresh_lobby()
                self.ui.switch_frame(FN_LOBBY)
            case "ROOM":
                # update room
                pass
            case "GAME_START":
                # maybe nothing?
                pass
            case "HAND":
                # update hand
                pass
            case "TURN":
                # update current turn, top card
                pass
            case "PLAYED":
                # show what other player did
                pass
            case "DRAWED":
                # show that other player drew
                pass
            case "SKIP":
                # maybe show sho is skipped
                pass
            case "CARDS":
                # add these cards to hand
                pass
            case "WIN":
                # show you are winner
                pass
            case "LOSE":
                # show you are loser
                pass
            case "PING":
                self.net.send_command(CMD_PONG)
            case "STATE":
                # change UI accordingly
                pass
            case "SLEEP":
                # show who is sleeping
                pass
            case "DEAD":
                # show who is dead
                pass
            case "AWAKE":
                # show who is awake
                pass
            case _:
                _ = messagebox.showerror("Unknown message received:",msg)

    def parse_rooms_message(self, msg: list[str]) -> None:
        try:
            count: int = int(msg[1])
            rooms: list[Room] = []

            beg_idx: int = 2 # skip cmd, count
            ris: int = 2 # room info size
            for i in range(beg_idx, beg_idx + count * ris, ris):
                rooms.append(
                    Room(
                        int(msg[i]),
                        msg[i+1]
                ))

            # replace with new rooms
            self.known_rooms_ = rooms

        except Exception as e:
            print(f"[PROTO] invalid rooms message received ({" ".join(msg)})\
            resulting in: {e}")
            _ = messagebox.showerror("Refresh", "Couldn't refresh.")
















