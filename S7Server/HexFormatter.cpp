#include "HexFormatter.h"

std::string HexFormatter::toHexString(const uint8_t* data, size_t length, bool spaceSeparated) {
    if (!data || length == 0) {
        return "";
    }

    std::ostringstream oss;
    for (size_t i = 0; i < length; ++i) {
        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
            << static_cast<int>(data[i]);
        if (spaceSeparated && i < length - 1) {
            oss << " ";
        }
    }
    return oss.str();
}

std::string HexFormatter::toHexDump(const uint8_t* data, size_t length, size_t bytesPerLine) {
    if (!data || length == 0) {
        return "";
    }

    std::ostringstream oss;
    
    for (size_t offset = 0; offset < length; offset += bytesPerLine) {
        // Print offset
        oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) 
            << offset << ": ";

        // Print hex bytes
        size_t lineLength = std::min(bytesPerLine, length - offset);
        for (size_t i = 0; i < bytesPerLine; ++i) {
            if (i < lineLength) {
                oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
                    << static_cast<int>(data[offset + i]) << " ";
            } else {
                oss << "   "; // Empty space for incomplete lines
            }

            // Add extra space in the middle for readability
            if (i == (bytesPerLine / 2) - 1) {
                oss << " ";
            }
        }

        // Print ASCII representation
        oss << " |";
        for (size_t i = 0; i < lineLength; ++i) {
            oss << toASCII(data[offset + i]);
        }
        oss << "|";

        // New line (except for last line)
        if (offset + bytesPerLine < length) {
            oss << "\n";
        }
    }

    return oss.str();
}

std::string HexFormatter::byteToHex(uint8_t byte) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) 
        << static_cast<int>(byte);
    return oss.str();
}

bool HexFormatter::isPrintable(uint8_t byte) {
    return byte >= 0x20 && byte <= 0x7E;
}

char HexFormatter::toASCII(uint8_t byte) {
    return isPrintable(byte) ? static_cast<char>(byte) : '.';
}
