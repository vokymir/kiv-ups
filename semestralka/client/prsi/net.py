import queue

Queue_Message = tuple[str, str|None]

class Net:
    def __init__(self, message_queue: queue.Queue[Queue_Message]) -> None:
        pass
