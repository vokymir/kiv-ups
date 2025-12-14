import tkinter as tk
from .client import PrsiClient


def main() -> None:
    root = tk.Tk()
    _app = PrsiClient(root)
    root.mainloop()


if __name__ == "__main__":
    main()
