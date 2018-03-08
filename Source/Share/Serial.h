#pragma once
#include <SFML/Network.hpp>
#include <functional>
#include "NetworkEvents.h"

// only handles primatives!
// somewhat low level class that allows easier serialization/deserialization
// features over using memcpy:
//   handles offset into the destination buffer automatically
//   less boilerplate code. Compare:
//   -> memcpy(dest+offset, (char *)&src, sizeof(src))
//   -> serialize(src)
//   This means fewer careless errors!
class Serial {
public:
    // used for error checking. never exceeded to avoid buffer overflow
    // (crashes program if exceeded)
    Serial(unsigned char *data, size_t len);
    
    // don't mix these within one object! (serialize and deserialize)
    template <typename T>
    void serialize(const T &v);
    
    template <typename T>
    void deserialize(T &v);
    
    size_t getOffset() const {return offset;}
    void resetOffset() {offset = 0;}
private:
    size_t length;
    size_t offset;
    unsigned char *data;
};

// abstraction over Serial for reading only.
// automatically calls necessary read calls and handles memory for
// all deserialization needs.
class ReadStream {
public:
    ReadStream(sf::TcpSocket &socket);
    bool isDone() const;
    
    template <typename T>
    bool deserialize(T &v);

private:
    bool hasReadAll = false;
    bool readOnce = false;
    
    constexpr static size_t MAX_PACKET = 2048;
    unsigned char networkData[MAX_PACKET];
    
    size_t received;
    size_t amountInBuffer = 0;
    
    sf::TcpSocket &socket;
    Serial serial;
};
