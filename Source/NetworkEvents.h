#pragma once
#include <cstdint>

enum NetworkEvent : int32_t {
    NONE = -1,
    // server to client events
    RECTANGLE_DELETE,
    RECTANGLE_UPDATE,
    
    // clients to server events
};
