#include "TagValueCodec.h"

float TagValueCodec::decodeReal(const uint8_t* buffer, int offset) {
    // S7 uses big-endian byte order, convert to little-endian (Windows x86/x64)
    uint8_t floatBytes[4];
    floatBytes[0] = buffer[offset + 3];  // Least significant byte
    floatBytes[1] = buffer[offset + 2];
    floatBytes[2] = buffer[offset + 1];
    floatBytes[3] = buffer[offset + 0];  // Most significant byte
    
    float result;
    std::memcpy(&result, floatBytes, 4);
    return result;
}

void TagValueCodec::encodeReal(uint8_t* buffer, int offset, float value) {
    // Convert float to bytes
    uint8_t floatBytes[4];
    std::memcpy(floatBytes, &value, 4);
    
    // Convert to big-endian for S7
    buffer[offset + 0] = floatBytes[3];  // Most significant byte
    buffer[offset + 1] = floatBytes[2];
    buffer[offset + 2] = floatBytes[1];
    buffer[offset + 3] = floatBytes[0];  // Least significant byte
}

uint32_t TagValueCodec::decodeDword(const uint8_t* buffer, int offset) {
    // S7 uses big-endian byte order
    uint32_t value = (static_cast<uint32_t>(buffer[offset + 0]) << 24) |
                     (static_cast<uint32_t>(buffer[offset + 1]) << 16) |
                     (static_cast<uint32_t>(buffer[offset + 2]) << 8)  |
                     (static_cast<uint32_t>(buffer[offset + 3]));
    return value;
}

void TagValueCodec::encodeDword(uint8_t* buffer, int offset, uint32_t value) {
    // Convert to big-endian for S7
    buffer[offset + 0] = static_cast<uint8_t>((value >> 24) & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>((value >> 16) & 0xFF);
    buffer[offset + 2] = static_cast<uint8_t>((value >> 8) & 0xFF);
    buffer[offset + 3] = static_cast<uint8_t>(value & 0xFF);
}

int16_t TagValueCodec::decodeInt(const uint8_t* buffer, int offset) {
    // S7 uses big-endian byte order
    int16_t value = (static_cast<int16_t>(buffer[offset + 0]) << 8) |
                    (static_cast<int16_t>(buffer[offset + 1]));
    return value;
}

void TagValueCodec::encodeInt(uint8_t* buffer, int offset, int16_t value) {
    // Convert to big-endian for S7
    buffer[offset + 0] = static_cast<uint8_t>((value >> 8) & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>(value & 0xFF);
}

bool TagValueCodec::decodeBool(const uint8_t* buffer, int offset, int bitPosition) {
    // Check if bit is set (bit 0 is LSB)
    uint8_t byte = buffer[offset];
    return (byte & (1 << bitPosition)) != 0;
}

void TagValueCodec::encodeBool(uint8_t* buffer, int offset, int bitPosition, bool value) {
    uint8_t mask = 1 << bitPosition;
    if (value) {
        // Set bit
        buffer[offset] |= mask;
    } else {
        // Clear bit
        buffer[offset] &= ~mask;
    }
}

uint32_t TagValueCodec::swapBytes32(uint32_t value) {
    return ((value & 0xFF000000) >> 24) |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x000000FF) << 24);
}

uint16_t TagValueCodec::swapBytes16(uint16_t value) {
    return ((value & 0xFF00) >> 8) |
           ((value & 0x00FF) << 8);
}
