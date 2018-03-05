#pragma once
#include <SFML/Network.hpp>
#include "NetworkEvents.h"

// only handles primatives!
class Serial {
public:
    // used for error checking. never exceeded to avoid buffer overflow
    Serial(unsigned char *data, size_t len);
    
    // don't mix these within one object! (serialize and deserialize)
    template <typename T>
    void serialize(const T &v);
    
    template <typename T>
    void deserialize(T &v);
    
    size_t getOffset() const {return offset;}
private:
    size_t length;
    size_t offset;
    unsigned char *data;
};


