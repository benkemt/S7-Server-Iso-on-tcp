#pragma once

#include "HttpServer.h"
#include "MemoryAccessor.h"
#include "HexFormatter.h"
#include "TagValueCodec.h"
#include "json/json.hpp"
#include <vector>

// Forward declarations
struct TagState;
enum class AreaType;
enum class DataType;

using json = nlohmann::json;

/*
 * API Endpoints - REST API handlers for web UI
 * 
 * Registers all HTTP endpoints with the HTTP server.
 * Provides access to memory areas, tag lists, and tag updates.
 */

class APIEndpoints {
public:
    // Register all endpoints with the HTTP server
    static void registerEndpoints(
        HttpServer* server,
        MemoryAccessor* memoryAccessor,
        std::vector<TagState>* tagStates
    );

private:
    // Helper to convert TagState to JSON
    static json tagStateToJson(const TagState& tag);
    
    // Helper to convert AreaType enum to string
    static std::string areaTypeToString(AreaType type);
    
    // Helper to convert DataType enum to string
    static std::string dataTypeToString(DataType type);
    
    // Helper to parse data type string to enum
    static DataType parseDataType(const std::string& str);
    
    // Helper to get tag address string
    static std::string getTagAddress(const TagState& tag);
};
