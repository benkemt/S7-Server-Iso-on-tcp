#include "APIEndpoints.h"
#include "WebUIContent.h"
#include "httplib/httplib.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdint>

// Replicate the enums and structures from main.cpp
// These must match exactly with the definitions in main.cpp
enum class AreaType {
    DB, INPUT, OUTPUT, MERKER, UNKNOWN
};

enum class DataType {
    REAL, DWORD, INT, BOOL, UNKNOWN
};

struct TagState {
    AreaType areaType;
    int dbNumber;
    int offset;
    int bitPosition;
    DataType dataType;
    double currentValue;
    double minValue;
    double maxValue;
    double echelon;
    int cycletime;
    bool increasing;
    std::chrono::steady_clock::time_point lastUpdateTime;
    uint8_t* dataPtr;
    bool manualOverride;
};

void APIEndpoints::registerEndpoints(
    HttpServer* server,
    MemoryAccessor* memoryAccessor,
    std::vector<TagState>* tagStates)
{
    auto* srv = server->getServer();
    
    // Serve web UI at root
    srv->Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content(WebUIContent::getHTML(), "text/html");
    });
    
    // Health check endpoint
    srv->Get("/api/health", [](const httplib::Request& req, httplib::Response& res) {
        json response = {
            {"status", "ok"},
            {"service", "S7 Server Web UI"},
            {"timestamp", std::chrono::system_clock::now().time_since_epoch().count()}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Get all Data Block numbers
    srv->Get("/api/memory/dbs", [memoryAccessor](const httplib::Request& req, httplib::Response& res) {
        auto dbNumbers = memoryAccessor->getDataBlockNumbers();
        json response = {
            {"dataBlocks", dbNumbers}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Get specific Data Block memory as hex
    srv->Get(R"(/api/memory/db/(\d+))", [memoryAccessor](const httplib::Request& req, httplib::Response& res) {
        int dbNumber = std::stoi(req.matches[1]);
        auto data = memoryAccessor->readDataBlock(dbNumber);
        
        if (data.empty()) {
            json error = {
                {"error", "Data Block not found"},
                {"dbNumber", dbNumber}
            };
            res.status = 404;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        std::string hexDump = HexFormatter::toHexDump(data.data(), data.size());
        std::string hexString = HexFormatter::toHexString(data.data(), data.size());
        
        json response = {
            {"dbNumber", dbNumber},
            {"size", data.size()},
            {"hexDump", hexDump},
            {"hexString", hexString}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Get memory area (I, Q, M, T, C)
    srv->Get(R"(/api/memory/area/([IQMTC]))", [memoryAccessor](const httplib::Request& req, httplib::Response& res) {
        std::string areaType = req.matches[1];
        std::vector<uint8_t> data;
        int size = 0;
        std::string areaName;
        
        if (areaType == "I") {
            data = memoryAccessor->readInputArea();
            size = memoryAccessor->getInputAreaSize();
            areaName = "Input (I)";
        } else if (areaType == "Q") {
            data = memoryAccessor->readOutputArea();
            size = memoryAccessor->getOutputAreaSize();
            areaName = "Output (Q)";
        } else if (areaType == "M") {
            data = memoryAccessor->readFlagArea();
            size = memoryAccessor->getFlagAreaSize();
            areaName = "Merker/Flags (M)";
        } else if (areaType == "T") {
            data = memoryAccessor->readTimerArea();
            size = memoryAccessor->getTimerAreaSize();
            areaName = "Timers (T)";
        } else if (areaType == "C") {
            data = memoryAccessor->readCounterArea();
            size = memoryAccessor->getCounterAreaSize();
            areaName = "Counters (C)";
        } else {
            json error = {{"error", "Invalid area type"}};
            res.status = 400;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        if (data.empty()) {
            json error = {
                {"error", "Area not found or empty"},
                {"areaType", areaType}
            };
            res.status = 404;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        std::string hexDump = HexFormatter::toHexDump(data.data(), data.size());
        std::string hexString = HexFormatter::toHexString(data.data(), data.size());
        
        json response = {
            {"areaType", areaType},
            {"areaName", areaName},
            {"size", size},
            {"hexDump", hexDump},
            {"hexString", hexString}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Get all tags with current values
    srv->Get("/api/tags", [tagStates, memoryAccessor](const httplib::Request& req, httplib::Response& res) {
        if (!tagStates || tagStates->empty()) {
            json response = {{"tags", json::array()}};
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        json tagsArray = json::array();
        
        // Lock memory accessor while reading all tag values
        auto lock = memoryAccessor->acquireLock();
        
        for (const auto& tag : *tagStates) {
            tagsArray.push_back(tagStateToJson(tag));
        }
        
        json response = {
            {"tags", tagsArray},
            {"count", tagsArray.size()}
        };
        res.set_content(response.dump(), "application/json");
    });
    
    // Update tag value
    srv->Post("/api/tags/update", [tagStates, memoryAccessor](const httplib::Request& req, httplib::Response& res) {
        if (!tagStates) {
            json error = {{"error", "Tag states not initialized"}};
            res.status = 500;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        // Parse request body
        json requestBody;
        try {
            requestBody = json::parse(req.body);
        } catch (const std::exception& e) {
            json error = {
                {"error", "Invalid JSON"},
                {"message", e.what()}
            };
            res.status = 400;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        // Get tag address and new value
        if (!requestBody.contains("address") || !requestBody.contains("value")) {
            json error = {{"error", "Missing 'address' or 'value' field"}};
            res.status = 400;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        std::string address = requestBody["address"];
        double newValue = requestBody["value"];
        
        // Find matching tag
        bool found = false;
        auto lock = memoryAccessor->acquireLock();
        
        for (auto& tag : *tagStates) {
            std::string tagAddr = getTagAddress(tag);
            if (tagAddr == address) {
                found = true;
                
                // Validate range
                if (newValue < tag.minValue || newValue > tag.maxValue) {
                    json error = {
                        {"error", "Value out of range"},
                        {"value", newValue},
                        {"min", tag.minValue},
                        {"max", tag.maxValue}
                    };
                    res.status = 400;
                    res.set_content(error.dump(), "application/json");
                    return;
                }
                
                // Update memory based on data type
                if (tag.dataPtr) {
                    switch (tag.dataType) {
                        case DataType::REAL:
                            TagValueCodec::encodeReal(tag.dataPtr, tag.offset, static_cast<float>(newValue));
                            break;
                        case DataType::DWORD:
                            TagValueCodec::encodeDword(tag.dataPtr, tag.offset, static_cast<uint32_t>(newValue));
                            break;
                        case DataType::INT:
                            TagValueCodec::encodeInt(tag.dataPtr, tag.offset, static_cast<int16_t>(newValue));
                            break;
                        case DataType::BOOL:
                            TagValueCodec::encodeBool(tag.dataPtr, tag.offset, tag.bitPosition, newValue != 0);
                            break;
                        default:
                            break;
                    }
                }
                
                // Update tag state
                tag.currentValue = newValue;
                tag.manualOverride = true;  // Disable auto-updates for this tag
                
                json response = {
                    {"success", true},
                    {"address", address},
                    {"newValue", newValue},
                    {"message", "Tag value updated successfully (auto-update disabled)"}
                };
                res.set_content(response.dump(), "application/json");
                return;
            }
        }
        
        if (!found) {
            json error = {
                {"error", "Tag not found"},
                {"address", address}
            };
            res.status = 404;
            res.set_content(error.dump(), "application/json");
        }
    });
    
    // Resume auto-update for a tag
    srv->Post("/api/tags/resume-auto", [tagStates](const httplib::Request& req, httplib::Response& res) {
        if (!tagStates) {
            json error = {{"error", "Tag states not initialized"}};
            res.status = 500;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        json requestBody;
        try {
            requestBody = json::parse(req.body);
        } catch (const std::exception& e) {
            json error = {{"error", "Invalid JSON"}};
            res.status = 400;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        if (!requestBody.contains("address")) {
            json error = {{"error", "Missing 'address' field"}};
            res.status = 400;
            res.set_content(error.dump(), "application/json");
            return;
        }
        
        std::string address = requestBody["address"];
        
        for (auto& tag : *tagStates) {
            if (getTagAddress(tag) == address) {
                tag.manualOverride = false;
                json response = {
                    {"success", true},
                    {"address", address},
                    {"message", "Auto-update resumed for tag"}
                };
                res.set_content(response.dump(), "application/json");
                return;
            }
        }
        
        json error = {{"error", "Tag not found"}};
        res.status = 404;
        res.set_content(error.dump(), "application/json");
    });
}

json APIEndpoints::tagStateToJson(const TagState& tag) {
    return {
        {"address", getTagAddress(tag)},
        {"areaType", areaTypeToString(tag.areaType)},
        {"dbNumber", tag.dbNumber},
        {"offset", tag.offset},
        {"bitPosition", tag.bitPosition},
        {"dataType", dataTypeToString(tag.dataType)},
        {"currentValue", tag.currentValue},
        {"minValue", tag.minValue},
        {"maxValue", tag.maxValue},
        {"echelon", tag.echelon},
        {"cycletime", tag.cycletime},
        {"increasing", tag.increasing},
        {"manualOverride", tag.manualOverride}
    };
}

std::string APIEndpoints::areaTypeToString(AreaType type) {
    switch (type) {
        case AreaType::DB: return "DB";
        case AreaType::INPUT: return "I";
        case AreaType::OUTPUT: return "Q";
        case AreaType::MERKER: return "M";
        default: return "UNKNOWN";
    }
}

std::string APIEndpoints::dataTypeToString(DataType type) {
    switch (type) {
        case DataType::REAL: return "REAL";
        case DataType::DWORD: return "DWORD";
        case DataType::INT: return "INT";
        case DataType::BOOL: return "BOOL";
        default: return "UNKNOWN";
    }
}

DataType APIEndpoints::parseDataType(const std::string& str) {
    if (str == "REAL") return DataType::REAL;
    if (str == "DWORD") return DataType::DWORD;
    if (str == "INT") return DataType::INT;
    if (str == "BOOL") return DataType::BOOL;
    return DataType::UNKNOWN;
}

std::string APIEndpoints::getTagAddress(const TagState& tag) {
    std::ostringstream oss;
    
    switch (tag.areaType) {
        case AreaType::DB:
            oss << "DB" << tag.dbNumber << "," << dataTypeToString(tag.dataType) << tag.offset;
            if (tag.dataType == DataType::BOOL) {
                oss << "." << tag.bitPosition;
            }
            break;
        case AreaType::INPUT:
            oss << "I" << tag.offset;
            if (tag.dataType == DataType::BOOL) {
                oss << "." << tag.bitPosition;
            }
            break;
        case AreaType::OUTPUT:
            oss << "Q" << tag.offset;
            if (tag.dataType == DataType::BOOL) {
                oss << "." << tag.bitPosition;
            }
            break;
        case AreaType::MERKER:
            oss << "M" << tag.offset;
            if (tag.dataType == DataType::BOOL) {
                oss << "." << tag.bitPosition;
            }
            break;
        default:
            oss << "UNKNOWN";
    }
    
    return oss.str();
}
