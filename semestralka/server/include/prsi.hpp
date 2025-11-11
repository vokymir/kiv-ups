#pragma once
/*
 * Master include for all public headers in PRSI project.
 * */

// ===== GAME =====
#include "prsi/game/Deck.hpp"
#include "prsi/game/Game.hpp"
#include "prsi/game/GameManager.hpp"
#include "prsi/game/Lobby.hpp"
#include "prsi/game/Room.hpp"

// ===== SERVER =====
#include "prsi/server/Protocol.hpp"
#include "prsi/server/Server.hpp"
#include "prsi/server/Session.hpp"
#include "prsi/server/SessionManager.hpp"

// ===== UTIL =====
#include "prsi/util/Config.hpp"
#include "prsi/util/Logger.hpp"

// ===== INTERFACES =====
#include "prsi/interfaces/IInterface.hpp"
