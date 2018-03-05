#pragma once
#include <cstdint>

enum NetworkEvent : int32_t {
    NONE = -1,
    // server to client events
    RECTANGLE_DELETE,
    RECTANGLE_UPDATE,
    
    // clients to server events
    REGISTER_PLAYER,
    LIST_GAMES,
    CREATE_GAME,
    JOIN_GAME,
    EXIT_GAME,
    UNREGISTER_PLAYER
};