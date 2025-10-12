import socket

HOST = '127.0.0.1'  # Server address (localhost)
PORT = 10000        # Server port

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    print(f"Connected to {HOST} on port {PORT}")

    # Step 1: send HELLO
    s.sendall(b"HELLO\n")
    print("Sent HELLO");

    # Step 2: receive NUM:<number>
    data = s.recv(1024).decode().strip()
    if not data.startswith("NUM:"):
        print("Protocol error:", data)
        exit(1)
    else:
        print(f"Got {data}")

    num = int(data.split(":")[1])
    answer = str(num * 2) + "\n"
    s.sendall(answer.encode())
    print(f"Sent answer: {answer}")

    # Step 3: receive final response (OK / WRONG / ERROR)
    result = s.recv(1024).decode().strip()
    print("Server response:", result)
