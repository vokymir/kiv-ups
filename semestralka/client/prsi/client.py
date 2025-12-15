from queue import Queue
from tkinter import messagebox
import queue
from prsi.net import QM_DISCONNECTED, QM_ERROR, QM_MESSAGE, Net, Queue_Message
from prsi.ui import Ui


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
        print(f"[PROTO] Received: {msg}")
        parts: list[str] = msg.split()
        if (not parts):
            return

