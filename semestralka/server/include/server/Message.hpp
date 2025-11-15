#pragma once

#include <string>
#include <variant>

namespace prsi::server {

// TODO: these messages are just placeholders, only Client/Server_Message should
// be used

// ===== CLIENT MESSAGES =====
struct CM_Connect {
  int fd_;
};

struct CM_Nick {
  int client_id_;
  std::string name_;
};

// ===== SERVER MESSAGES =====
struct SM_Accept {
  int client_id_;
};

struct SM_Decline {};

// ===== MESSAGES =====
using Client_Message = std::variant<CM_Connect, CM_Nick>;
using Server_Message = std::variant<SM_Accept, SM_Decline>;
// TODO: is this even useful anywhere?
using Message = std::variant<Client_Message, Server_Message>;

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
