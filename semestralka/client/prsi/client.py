from queue import Queue
from tkinter import messagebox
import queue
from prsi.net import QM_DISCONNECTED, QM_ERROR, QM_MESSAGE, Net, Queue_Message
from prsi.ui import Ui
from prsi.config import CMD_PONG


class Client:
    """
    The controller, have net & ui and actually do all the stuff.
    """
    def __init__(self) -> None:
        # message queue: for getting messages out of net
        self.mq: Queue[Queue_Message] = queue.Queue()

        # network part of client - talk via queue
        self.net: Net = Net(self.mq)

        # ui
        self.ui: Ui = Ui(self)

        _ = self.ui.after(100, self.process_incoming_messages)

    def run(self) -> None:
        self.ui.mainloop()

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
                self.ui.switch_frame("login")

            elif (qm_type == QM_DISCONNECTED):
                self.ui.switch_frame("login")

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
                return
            case "ROOMS":
                # update lobby
                pass
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















