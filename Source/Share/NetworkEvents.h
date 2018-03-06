#pragma once
#include <cstdint>

enum NetworkEvent : int32_t {
    NONE = -1,
    // server to client events
    RECTANGLE_DELETE,
    RECTANGLE_UPDATE,
    
    START_GAME, // YAY!
    
    
    // clients to server events
    CREATE_RECTANGLE,
    
    REGISTER_PLAYER,
    LIST_GAMES,
    CREATE_GAME,
    JOIN_GAME,
    EXIT_GAME,
    UNREGISTER_PLAYER,
    
    READY_GAME, // both players must tell server they are ready before game launches
    
    
    // both ways
    CHAT_LOBBY,
};
