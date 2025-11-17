#pragma once

#include "game/Card.hpp"
#include "game/Player.hpp"
#include "game/Prsi_Game.hpp"
#include "game/Room.hpp"
#include <string>
#include <variant>
#include <vector>

namespace prsi::server {

// ===== SERVER MESSAGES =====

// == PERIODIC ==
// ping client
struct SM_Ping {};

// == CLIENT CONNECTED ==
// server wants nick, if invalid is given = disconnect
struct SM_Want_Nick {};

// == CLIENT LOBBY ==
// send all rooms & their info, expects create/join message
// may be sent multiple times before receiving response, because something
// was updated (e.g. new room created)
struct SM_Rooms {
  std::vector<const game::Room *> rooms;
};

// == CLIENT ROOM ==
// send room info (multiple times if updates) after joined room
// expects nothing or leave message
struct SM_Room {
  game::Room *room;
};

// == CLIENT WAIT ON TURN ==
// send complete game snapshot - only on game start or after brief disconnect
struct SM_Game_Snapshot {
  game::Prsi_Game *game;
};

// notify clients someone in game is disconnected
// expects no response
struct SM_Game_Brief_Disconnect {
  game::Player *who;
};
struct SM_Game_Reconnect {
  game::Player *who;
};
struct SM_Game_Terminal_Disconnect {
  game::Player *who;
};

// incremental changes in game state
// expects no response
struct SM_Game_Played {
  game::Card what;
};
struct SM_Game_Drew {};
struct SM_Game_Passed {
  int how_many; // if SEVEN then number, if ESO then -1
};

// == CLIENT PLAYING ==
// tell client to play
// expect play/draw/pass - if operation invalid, disconnect
struct SM_Play {};

// == CLIENT GAME FINISHED ==
// expects Go To Lobby message
struct SM_Game_Finished {
  std::vector<game::Player *> leaderboard;
};

// ===== CLIENT MESSAGES =====

// == PERIODIC ==
// respond to ping from server
struct CM_Pong {};

// == CLIENT CONNECTED ==
// send nick to server
struct CM_Nick {
  std::string nick;
};

// == CLIENT LOBBY ==
struct CM_Join_Room {
  int id;
};
struct CM_Create_Room {};

// == CLIENT ROOM ==
struct CM_Leave_Room {};

// == CLIENT WAIT ON TURN ==

// == CLIENT PLAYING ==
struct CM_Play {
  game::Card card;
};
struct CM_Draw {};
struct CM_Pass {};

// == CLIENT GAME FINISHED ==
struct CM_Goto_Lobby {};

// ===== MESSAGES =====
using Server_Message =
    std::variant<SM_Ping, SM_Want_Nick, SM_Rooms, SM_Room, SM_Game_Snapshot,
                 SM_Game_Brief_Disconnect, SM_Game_Reconnect,
                 SM_Game_Terminal_Disconnect, SM_Game_Played, SM_Game_Drew,
                 SM_Game_Passed, SM_Play, SM_Game_Finished>;
using Client_Message =
    std::variant<CM_Pong, CM_Nick, CM_Join_Room, CM_Create_Room, CM_Leave_Room,
                 CM_Play, CM_Draw, CM_Pass, CM_Goto_Lobby>;

} // namespace prsi::server
