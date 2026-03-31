#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

/*
 * HexFormatter - Utilities for formatting byte arrays as hexadecimal strings
 * 
 * Provides functions to convert memory areas into human-readable hex dumps
 * with offsets, hex values, and ASCII preview (similar to hexdump -C)
 */

class HexFormatter {
public:
    /*
     * Format bytes as hex string with optional ASCII preview
     * Format: "41 F2 00 3F"
     */
    static std::string toHexString(const uint8_t* data, size_t length, bool spaceSeparated = true);

    /*
     * Format bytes as complete hex dump with offsets and ASCII
     * Format:
     * 0000: 41 F2 00 3F 80 00 00 00  00 00 00 00 FF FF FF FF  |A...?...........|
     * 0010: 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F  |................|
     */
    static std::string toHexDump(const uint8_t* data, size_t length, size_t bytesPerLine = 16);

    /*
     * Format single byte as two hex characters
     * Format: "A5"
     */
    static std::string byteToHex(uint8_t byte);

    /*
     * Check if byte is printable ASCII character
     */
    static bool isPrintable(uint8_t byte);

    /*
     * Get ASCII representation of byte (or '.' for non-printable)
     */
    static char toASCII(uint8_t byte);
};
