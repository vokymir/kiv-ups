#pragma once

#include <variant>

namespace prsi::server {

// ===== CLIENT MESSAGES =====
struct CM_Pong {};

// ===== SERVER MESSAGES =====
struct SM_Ping {};
struct SM_Someone_Disconnected {
  int fd_; // who is disconnected
};

// ===== MESSAGES =====
using Client_Message = std::variant<CM_Pong>;
using Server_Message = std::variant<SM_Ping, SM_Someone_Disconnected>;

/* HOW TO USE

void handle_message(const Message& msg) {
    std::visit([](const auto& m) {
        using T = std::decay_t<decltype(m)>;

        if constexpr (std::is_same_v<T, ConnectMsg>) {
            // handle connect
            std::cout << "Player " << m.player_id
                      << " connected as " << m.nickname << "\n";
        }
        else if constexpr (std::is_same_v<T, PlayMsg>) {
            // handle play
            std::cout << "Player " << m.player_id
                      << " played card " << m.card_id << "\n";
        }
        else if constexpr (std::is_same_v<T, ErrorMsg>) {
            // handle errors
            std::cerr << "Error: " << m.description << "\n";
        }
    }, msg);
}

 * */

} // namespace prsi::server
