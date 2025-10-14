import socket
from time import sleep

HOST = '147.228.67.113'
PORT = 4242

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    print(f"Connected to {HOST} on port {PORT}")

    s.send(b"KIVUPSnick000705javok")

    sleep(1)

    s.send(b"KIVUPSchat0007004ahoj")
