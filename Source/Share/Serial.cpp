#include "Serial.h"
#include <cassert>
#include <cstring>
#include <string>
#include <cstdint>
#include <algorithm>
#include <iostream>

Serial::Serial(unsigned char *data, size_t len) : length(len), offset(0), data(data) {}

template <typename T>
void Serial::serialize(const T &v) {
    assert(offset+sizeof(v) <= length);
    
    memcpy(data+offset, (char *)&v, sizeof(v));

    offset += sizeof(v);
}


template <>
void Serial::serialize(const std::string &v) {
    int32_t s = v.size();
    serialize(s);
    
    assert(offset+s <= length);
    
    memcpy(data+offset, v.data(), s);

    offset += s;
}


template <typename T>
void Serial::deserialize(T &v) {
    assert(offset+sizeof(v) <= length);
    
    memcpy((char *)&v, data+offset, sizeof(v));

    offset += sizeof(v);
}

template <>
void Serial::deserialize(std::string &v) {
    int32_t s;
    deserialize(s);
    
    assert(offset+s <= length);
    v = std::string(reinterpret_cast<char *>(data+offset), s);

    offset += s;
}


template void Serial::serialize<NetworkEvent>(const NetworkEvent &);

template void Serial::serialize<int8_t>(const int8_t &);
template void Serial::serialize<int16_t>(const int16_t &);
template void Serial::serialize<int32_t>(const int32_t &);

template void Serial::serialize<uint8_t>(const uint8_t &);
template void Serial::serialize<uint16_t>(const uint16_t &);
template void Serial::serialize<uint32_t>(const uint32_t &);

template void Serial::serialize<float>(const float &);






template void Serial::deserialize<NetworkEvent>(NetworkEvent &);

template void Serial::deserialize<int8_t>(int8_t &);
template void Serial::deserialize<int16_t>(int16_t &);
template void Serial::deserialize<int32_t>(int32_t &);

template void Serial::deserialize<uint8_t>(uint8_t &);
template void Serial::deserialize<uint16_t>(uint16_t &);
template void Serial::deserialize<uint32_t>(uint32_t &);

template void Serial::deserialize<float>(float &);






ReadStream::ReadStream(sf::TcpSocket &socket) :
    socket(socket), serial(networkData, MAX_PACKET) {}

template <typename T>
bool ReadStream::deserialize(T &v) {
    if(isDone()) return false;
    
    // get more data if necessary
    if(!hasReadAll) {
        size_t startingPoint = 0; // place to read into networkData
        if(readOnce) {
            // don't waste precious cpu time for anything less than MAX_PACKET/2
            if(serial.getOffset() < MAX_PACKET/2) {
                serial.deserialize(v);
                return true;
            }
            
            // work next packet amount into this one by doing a lot of shifting
            size_t amount = serial.getOffset();
            startingPoint = MAX_PACKET - amount;
            
            std::rotate(networkData, networkData+amount, networkData+MAX_PACKET);
            amountInBuffer = MAX_PACKET-amount;
            
            serial.resetOffset();
        }
        
        // read in as much as necessary/possible
        sf::Socket::Status status;
        while((status = socket.receive(networkData+startingPoint, MAX_PACKET-startingPoint, received)) == sf::Socket::Partial) {
            startingPoint += received;
        }
        
        if(status == sf::Socket::NotReady) return false;
        if(status == sf::Socket::Disconnected) return false;
        
        readOnce = true;
        
        // determine how full the buffer is
        received += startingPoint;
        
        assert(status == sf::Socket::Done);
        
        if(received < MAX_PACKET) {
            hasReadAll = true;
        }
        
        // update the amount of the buffer that is filled
        if(received > amountInBuffer) {
            amountInBuffer = received;
        }
    }
    
    // all for this!
    serial.deserialize(v);
    return true;
}


bool ReadStream::isDone() const {
    // is the amount of stuff in the buffer less than the amount of stuff deserialized?
    return readOnce && amountInBuffer <= serial.getOffset();
}




template bool ReadStream::deserialize<NetworkEvent>(NetworkEvent &);

template bool ReadStream::deserialize<int8_t>(int8_t &);
template bool ReadStream::deserialize<int16_t>(int16_t &);
template bool ReadStream::deserialize<int32_t>(int32_t &);

template bool ReadStream::deserialize<uint8_t>(uint8_t &);
template bool ReadStream::deserialize<uint16_t>(uint16_t &);
template bool ReadStream::deserialize<uint32_t>(uint32_t &);

template bool ReadStream::deserialize<float>(float &);
template bool ReadStream::deserialize<std::string>(std::string &);


