#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

// LLM spitted, now read-through and redo
class SocketRAII {
public:
    explicit SocketRAII(int fd = -1) : fd_(fd) {}
    ~SocketRAII() { if (fd_ >= 0) ::close(fd_); }

    SocketRAII(const SocketRAII&) = delete;
    SocketRAII& operator=(const SocketRAII&) = delete;

    SocketRAII(SocketRAII&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    SocketRAII& operator=(SocketRAII&& other) noexcept {
        if (this != &other) {
            if (fd_ >= 0) ::close(fd_);
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    [[nodiscard]] int get() const { return fd_; }
    [[nodiscard]] bool valid() const { return fd_ >= 0; }

private:
    int fd_;
};

int main() {
    try {
        // Create listening socket
        SocketRAII server_socket(::socket(AF_INET, SOCK_STREAM, 0));
        if (!server_socket.valid())
            throw std::runtime_error("Socket creation failed");

        int reuse = 1;
        if (::setsockopt(server_socket.get(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
            std::cerr << "Warning: setsockopt(SO_REUSEADDR) failed\n";

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(10000);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if (::bind(server_socket.get(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) != 0)
            throw std::runtime_error("Bind failed");

        std::cout << "Bind - OK\n";

        if (::listen(server_socket.get(), 5) != 0)
            throw std::runtime_error("Listen failed");

        std::cout << "Listen - OK\n";

        // FD sets for select()
        fd_set client_socks;
        FD_ZERO(&client_socks);
        FD_SET(server_socket.get(), &client_socks);

        int max_fd = server_socket.get();

        std::cout << "Server running on port 10000...\n";

        while (true) {
            fd_set readfds = client_socks; // copy for select
            int activity = ::select(max_fd + 1, &readfds, nullptr, nullptr, nullptr);

            if (activity < 0) {
                std::cerr << "Select error\n";
                break;
            }

            for (int fd = 3; fd <= max_fd; ++fd) {
                if (FD_ISSET(fd, &readfds)) {
                    if (fd == server_socket.get()) {
                        // New connection
                        sockaddr_in client_addr{};
                        socklen_t len = sizeof(client_addr);
                        int client_fd = ::accept(server_socket.get(), reinterpret_cast<sockaddr*>(&client_addr), &len);
                        if (client_fd >= 0) {
                            FD_SET(client_fd, &client_socks);
                            if (client_fd > max_fd) max_fd = client_fd;
                            std::cout << "New client connected (fd=" << client_fd << ")\n";
                        }
                    } else {
                        // Existing client
                        int bytes_available = 0;
                        ::ioctl(fd, FIONREAD, &bytes_available);

                        if (bytes_available > 0) {
                            char c;
                            ssize_t received = ::recv(fd, &c, 1, 0);
                            if (received > 0) {
                                std::cout << "Received: " << c << " (from fd=" << fd << ")\n";
                            }
                        } else {
                            // Client disconnected
                            ::close(fd);
                            FD_CLR(fd, &client_socks);
                            std::cout << "Client (fd=" << fd << ") disconnected\n";
                        }
                    }
                }
            }
        }

    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
