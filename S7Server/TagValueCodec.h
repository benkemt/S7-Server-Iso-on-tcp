#pragma once

#include <cstdint>
#include <cstring>

/*
 * TagValueCodec - Encoder/Decoder for S7 data types
 * 
 * Converts between S7 wire format (big-endian) and native C++ types.
 * Supports REAL (float), DWORD (uint32), INT (int16), and BOOL.
 */

class TagValueCodec {
public:
    // Decode S7 REAL (big-endian IEEE 754 float) from bytes
    static float decodeReal(const uint8_t* buffer, int offset = 0);

    // Encode float to S7 REAL format
    static void encodeReal(uint8_t* buffer, int offset, float value);

    // Decode S7 DWORD (big-endian uint32) from bytes
    static uint32_t decodeDword(const uint8_t* buffer, int offset = 0);

    // Encode uint32 to S7 DWORD format
    static void encodeDword(uint8_t* buffer, int offset, uint32_t value);

    // Decode S7 INT (big-endian int16) from bytes
    static int16_t decodeInt(const uint8_t* buffer, int offset = 0);

    // Encode int16 to S7 INT format
    static void encodeInt(uint8_t* buffer, int offset, int16_t value);

    // Decode S7 BOOL from byte and bit position
    static bool decodeBool(const uint8_t* buffer, int offset, int bitPosition);

    // Encode bool to S7 BOOL format
    static void encodeBool(uint8_t* buffer, int offset, int bitPosition, bool value);

    // Helper: Swap byte order (for endian conversion)
    static uint32_t swapBytes32(uint32_t value);
    static uint16_t swapBytes16(uint16_t value);
};
