#include "Serial.h"
#include <cassert>
#include <cstring>
#include <cstdint>

Serial::Serial(unsigned char *data, size_t len) : length(len), offset(0), data(data) {}

template <typename T>
void Serial::serialize(const T &v) {
    assert(offset+sizeof(v) <= length);
    
    memcpy(data+offset, (char *)&v, sizeof(v));

    offset += sizeof(v);
}


template <typename T>
void Serial::deserialize(T &v) {
    assert(offset+sizeof(v) <= length);
    
    memcpy((char *)&v, data+offset, sizeof(v));

    offset += sizeof(v);
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

