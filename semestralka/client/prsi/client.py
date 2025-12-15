from queue import Queue
from tkinter import messagebox
import queue
from prsi.net import Net, Queue_Message
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

        self.ui.after(100, self.process_incoming_messages)

    def run(self) -> None:
        self.ui.mainloop()

    def process_incoming_messages(self) -> None:
        pass
