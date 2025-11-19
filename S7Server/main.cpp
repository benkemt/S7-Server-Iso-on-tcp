/*
 * S7 Server ISO-on-TCP Implementation
 * Using Snap7 Library for Node-RED Testing
 * 
 * This server creates a Siemens S7 compatible server using ISO-on-TCP protocol
 * that can be used for testing Node-RED S7 applications.
 * 
 * Features:
 * - Dynamic tag value updates based on CSV configuration
 * - Cycletime-based scheduling for value changes
 * - Sawtooth pattern value generation (min -> max -> min)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <csignal>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>
#include "snap7.h"

// Global server instance
S7Object S7Server = 0;
bool ServerRunning = true;

// Constants
const int REAL_SIZE = 4;  // S7 REAL data type size in bytes

// Structure to hold CSV configuration entry
struct CSVConfigEntry {
    int dbNumber;
    int offset;
    float minValue;
    float maxValue;
    float echelon;
    int cycletime;
};

// Structure to hold tag state for dynamic updates
struct TagState {
    int dbNumber;
    int offset;
    float currentValue;
    float minValue;
    float maxValue;
    float echelon;
    int cycletime;
    bool increasing;  // true = increasing, false = decreasing
    std::chrono::steady_clock::time_point lastUpdateTime;
    byte* dataPtr;  // Pointer to the data block memory
};

// Structure to hold Data Block information
struct DataBlock {
    int number;
    int size;
    byte* data;
};

// Signal handler for graceful shutdown
void SignalHandler(int signal) {
  std::cout << "\nShutdown signal received. Stopping server..." << std::endl;
    ServerRunning = false;
}

// Event callback function
void S7API EventCallback(void* usrPtr, PSrvEvent PEvent, int Size) {
    // Log server events
    std::string EventText;
    bool shouldLog = true;

    switch (PEvent->EvtCode) {
		case evcServerStarted:
			EventText = "Server started";
			break;
		case evcServerStopped:
			EventText = "Server stopped";
			break;
		case evcClientAdded:
			EventText = "Client connected";
			break;
		case evcClientDisconnected:
			EventText = "Client disconnected";
			break;
		case evcPDUincoming:
			EventText = "PDU incoming";
			break;
		case evcDataRead:
			// Uncomment to hide frequent data read events
			// shouldLog = false;
			EventText = "Data read";
			break;
		case evcDataWrite:
			EventText = "Data write";
			break;
		case evcNegotiatePDU:
			EventText = "Negotiate PDU - PDU Size: " + std::to_string(PEvent->EvtParam1) + " bytes";
			break;
		case evcReadSZL:
			EventText = "Read SZL";
			break;
		case evcClock:
			EventText = "Clock";
			break;
		case evcUpload:
			EventText = "Upload";
			break;
		case evcDownload:
			EventText = "Download";
			break;
		case evcDirectory:
			EventText = "Directory";
			break;
		case evcSecurity:
			EventText = "Security";
			break;
		case evcControl:
			EventText = "Control";
			break;
		default:
		// Only log truly unknown events
			if (PEvent->EvtCode != 0) {
				EventText = "Other event";
			} else {
				return; // Skip logging null events
			}
			break;
		}
    
    if (shouldLog) {
        std::cout << "[EVENT] " << EventText << " (Code: " << PEvent->EvtCode << ")" << std::endl;
    }
}

// Read event callback
void S7API ReadEventCallback(void* usrPtr, PSrvEvent PEvent, int Size) {
    // Area codes: PE=0x81, PA=0x82, MK=0x83, DB=0x84, CT=0x1C, TM=0x1D
    const char* areaName = "Unknown";
    if (PEvent->EvtParam1 == 0x84) areaName = "DB";
    else if (PEvent->EvtParam1 == 0x81) areaName = "I";
    else if (PEvent->EvtParam1 == 0x82) areaName = "Q";
    else if (PEvent->EvtParam1 == 0x83) areaName = "M";
    else if (PEvent->EvtParam1 == 0x1C) areaName = "C";
    else if (PEvent->EvtParam1 == 0x1D) areaName = "T";
    
    int byteCount = PEvent->EvtParam4;
    int realCount = (byteCount >= 4) ? byteCount / 4 : 0;
    
    std::cout << "[READ] Area: " << areaName << " (0x" << std::hex << PEvent->EvtParam1 << std::dec << ")"
    << ", DBNum/Start: " << PEvent->EvtParam2 
	<< ", Offset: " << PEvent->EvtParam3 
	<< ", Size: " << PEvent->EvtParam4 << " bytes";
    
    if (PEvent->EvtParam1 == 0x84 && realCount > 0) {
        std::cout << " (" << realCount << " REALs)";
    }
    
    std::cout << std::endl;
}

// Read/Write area callback (replaces separate write callback in new API)
// NOTE: This callback is intentionally disabled (see line 305).
// When enabled, it intercepts ALL read/write operations but was not implementing
// actual data transfer, causing all operations to fail silently.
// Snap7 handles read/write operations automatically using internal buffers
// when no callback is registered.
int S7API RWAreaCallback(void* usrPtr, int Sender, int Operation, PS7Tag PTag, void* pUsrData) {
    if (Operation == OperationWrite) {
        std::cout << "[WRITE] Area: " << PTag->Area 
       << ", Start: " << PTag->Start 
      << ", Size: " << PTag->Size << std::endl;
    }
    return 0; // Success
}

// Helper function to convert float to S7 REAL format (big-endian IEEE 754)
// Note: This function assumes the host system uses little-endian byte order (x86/x64 Windows).
// The caller is responsible for ensuring the buffer has sufficient space (offset + 4 bytes).
void SetReal(byte* buffer, int offset, float value) {
    // Convert float to bytes
    byte* floatBytes = reinterpret_cast<byte*>(&value);
    
    // S7 uses big-endian byte order, but Windows x86/x64 systems use little-endian
    // We need to reverse the byte order
    buffer[offset + 0] = floatBytes[3];  // Most significant byte
    buffer[offset + 1] = floatBytes[2];
    buffer[offset + 2] = floatBytes[1];
    buffer[offset + 3] = floatBytes[0];  // Least significant byte
}

// Helper function to read float from S7 REAL format (big-endian IEEE 754)
float GetReal(byte* buffer, int offset) {
    // S7 uses big-endian byte order, convert to little-endian
    byte floatBytes[4];
    floatBytes[3] = buffer[offset + 0];  // Most significant byte
    floatBytes[2] = buffer[offset + 1];
    floatBytes[1] = buffer[offset + 2];
    floatBytes[0] = buffer[offset + 3];  // Least significant byte
    
    return *reinterpret_cast<float*>(floatBytes);
}

// Parse CSV tag format "DB<number>,REAL<offset>"
bool ParseTag(const std::string& tag, int& dbNumber, int& offset) {
    // Remove quotes if present
    std::string cleanTag = tag;
    cleanTag.erase(std::remove(cleanTag.begin(), cleanTag.end(), '\"'), cleanTag.end());
    
    // Find DB and REAL positions
    size_t dbPos = cleanTag.find("DB");
    size_t realPos = cleanTag.find("REAL");
    
    if (dbPos == std::string::npos || realPos == std::string::npos) {
        return false;
    }
    
    // Extract DB number (between "DB" and ",")
    size_t commaPos = cleanTag.find(',');
    if (commaPos == std::string::npos) {
        return false;
    }
    
    try {
        std::string dbNumStr = cleanTag.substr(dbPos + 2, commaPos - dbPos - 2);
        dbNumber = std::stoi(dbNumStr);
        
        // Extract offset (after "REAL")
        std::string offsetStr = cleanTag.substr(realPos + 4);
        offset = std::stoi(offsetStr);
        
        return true;
    } catch (...) {
        return false;
    }
}

// Helper function to parse CSV line with quoted fields
std::vector<std::string> ParseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        if (c == '\"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    
    // Add the last field
    fields.push_back(field);
    
    return fields;
}

// Load CSV configuration file
std::vector<CSVConfigEntry> LoadCSVConfig(const std::string& filename) {
    std::vector<CSVConfigEntry> entries;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "WARNING: Could not open CSV file '" << filename << "'. Using default configuration." << std::endl;
        return entries;
    }
    
    std::string line;
    bool firstLine = true;
    
    while (std::getline(file, line)) {
        // Skip header line
        if (firstLine) {
            firstLine = false;
            continue;
        }
        
        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }
        
        // Parse CSV line with proper quote handling
        std::vector<std::string> fields = ParseCSVLine(line);
        
        if (fields.size() >= 5) {
            CSVConfigEntry entry;
            
            // Parse tag to get DB number and offset
            if (!ParseTag(fields[0], entry.dbNumber, entry.offset)) {
                std::cerr << "WARNING: Failed to parse tag: " << fields[0] << std::endl;
                continue;
            }
            
            try {
                entry.minValue = std::stof(fields[1]);
                entry.maxValue = std::stof(fields[2]);
                entry.echelon = std::stof(fields[3]);
                entry.cycletime = std::stoi(fields[4]);
                
                entries.push_back(entry);
            } catch (...) {
                std::cerr << "WARNING: Failed to parse values for tag: " << fields[0] << std::endl;
                continue;
            }
        }
    }
    
    file.close();
    std::cout << "Loaded " << entries.size() << " entries from CSV configuration." << std::endl;
    return entries;
}

// Generate random float value within range
float GenerateRandomValue(float minValue, float maxValue) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(minValue, maxValue);
    return dis(gen);
}

// Create and initialize Data Blocks from CSV configuration
std::vector<DataBlock> CreateDataBlocksFromCSV(const std::vector<CSVConfigEntry>& entries) {
    std::map<int, int> dbSizes; // DB number -> required size
    std::vector<DataBlock> dataBlocks;
    
    // First pass: determine required size for each DB
    for (const auto& entry : entries) {
        int requiredSize = entry.offset + REAL_SIZE;
        if (dbSizes.find(entry.dbNumber) == dbSizes.end()) {
            dbSizes[entry.dbNumber] = requiredSize;
        } else {
            dbSizes[entry.dbNumber] = std::max(dbSizes[entry.dbNumber], requiredSize);
        }
    }
    
    // Second pass: allocate Data Blocks and reserve capacity to prevent reallocation
    dataBlocks.reserve(dbSizes.size());  // Reserve capacity to prevent pointer invalidation
    for (const auto& pair : dbSizes) {
        DataBlock db;
        db.number = pair.first;
        db.size = pair.second;
        db.data = new byte[db.size]();
        
        std::cout << "Allocated DB" << db.number << ": " << db.size << " bytes" << std::endl;
        dataBlocks.push_back(db);
    }
    
    // Build map for fast lookup (safe now that vector won't reallocate)
    std::map<int, DataBlock*> dbMap;
    for (auto& db : dataBlocks) {
        dbMap[db.number] = &db;
    }
    
    // Third pass: initialize values from CSV using map for fast lookup
    // Start values at minimum to begin the increasing cycle
    for (const auto& entry : entries) {
        auto it = dbMap.find(entry.dbNumber);
        if (it != dbMap.end()) {
			DataBlock* db = it->second;

            float value = entry.minValue;  // Start at minimum value

			SetReal(db->data, entry.offset, value);
            std::cout << "  DB" << db->number << ".REAL" << entry.offset << " = " << value << " (range: " << entry.minValue << " to " << entry.maxValue << ")" << std::endl;
        }
    }
    
    // Summary: Show all created Data Blocks
    std::cout << "\nData Block Summary:" << std::endl;
    std::cout << "===================" << std::endl;
    for (const auto& db : dataBlocks) {
   std::cout << "  DB" << db.number << ": " << db.size << " bytes" << std::endl;
    }
 std::cout << "Total Data Blocks: " << dataBlocks.size() << std::endl;
    std::cout << "===================" << std::endl;
    
    return dataBlocks;
}

// Display server configuration
void DisplayConfig(const std::vector<DataBlock>& dataBlocks) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "S7 Server Configuration:" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Protocol: ISO-on-TCP" << std::endl;
    std::cout << "Port: 102" << std::endl;
    std::cout << "Data Blocks:" << std::endl;
    
    for (const auto& db : dataBlocks) {
        std::cout << "  - DB" << db.number << ": " << db.size << " bytes" << std::endl;
    }
    
    std::cout << "Inputs (I):  256 bytes" << std::endl;
    std::cout << "Outputs (Q): 256 bytes" << std::endl;
    std::cout << "Flags (M):   256 bytes" << std::endl;
    std::cout << "Timers (T):  512 bytes" << std::endl;
    std::cout << "Counters (C): 512 bytes" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// Display server status
void DisplayStatus(S7Object Server) {
	int ServerStatus, CpuStatus, ClientsCount;
	int Result = Srv_GetStatus(Server, &ServerStatus, &CpuStatus, &ClientsCount);

	if (Result == 0) {
		std::cout << "Server Status: " << (ServerStatus == 1 ? "RUNNING" : "STOPPED") << std::endl;
		std::cout << "Connected Clients: " << ClientsCount << std::endl;
	}
}

// Initialize tag states from CSV configuration and data blocks
std::vector<TagState> InitializeTagStates(const std::vector<CSVConfigEntry>& entries, 
                                          const std::vector<DataBlock>& dataBlocks) {
    std::vector<TagState> tagStates;
    
    // Build map for fast DB lookup
    std::map<int, const DataBlock*> dbMap;
    for (const auto& db : dataBlocks) {
        dbMap[db.number] = &db;
    }
    
    // Create tag state for each CSV entry
    for (const auto& entry : entries) {
        auto it = dbMap.find(entry.dbNumber);
        if (it != dbMap.end()) {
            TagState state;
            state.dbNumber = entry.dbNumber;
            state.offset = entry.offset;
            state.currentValue = entry.minValue;  // Start at minimum
            state.minValue = entry.minValue;
            state.maxValue = entry.maxValue;
            state.echelon = entry.echelon;
            state.cycletime = entry.cycletime;
            state.increasing = true;  // Start by increasing
            state.lastUpdateTime = std::chrono::steady_clock::now();
            state.dataPtr = it->second->data;
            
            tagStates.push_back(state);
        }
    }
    
    std::cout << "\nInitialized " << tagStates.size() << " tag states for dynamic updates." << std::endl;
    return tagStates;
}

// Update tag values based on cycletime and echelon
void UpdateTagValues(std::vector<TagState>& tagStates) {
    auto currentTime = std::chrono::steady_clock::now();
    
    for (auto& tag : tagStates) {
        // Calculate elapsed time since last update in milliseconds
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - tag.lastUpdateTime).count();
        
        // Check if it's time to update this tag based on cycletime
        if (elapsed >= tag.cycletime) {
            // Update the value based on direction and echelon
            if (tag.increasing) {
                tag.currentValue += tag.echelon;
                
                // Check if we've reached or exceeded the max value
                if (tag.currentValue >= tag.maxValue) {
                    tag.currentValue = tag.maxValue;
                    tag.increasing = false;  // Switch to decreasing
                }
            } else {
                tag.currentValue -= tag.echelon;
                
                // Check if we've reached or gone below the min value
                if (tag.currentValue <= tag.minValue) {
                    tag.currentValue = tag.minValue;
                    tag.increasing = true;  // Switch to increasing
                }
            }
            
            // Write the new value to the data block
            SetReal(tag.dataPtr, tag.offset, tag.currentValue);
            
            // Update last update time
            tag.lastUpdateTime = currentTime;
        }
    }
}

// Helper function to cleanup allocated memory
void CleanupResources(std::vector<DataBlock>& dataBlocks, byte* IArea, byte* QArea, 
                     byte* MArea, byte* TArea, byte* CArea) {
    for (auto& db : dataBlocks) {
        delete[] db.data;
    }
    delete[] IArea;
    delete[] QArea;
    delete[] MArea;
    delete[] TArea;
 delete[] CArea;
}

// Diagnostic function to verify DB area accessibility
bool VerifyDBAreaAccessible(S7Object server, int dbNumber, int size) {
    // Try to lock the area for verification
    void* pUsrData = nullptr;
    int Result = Srv_LockArea(server, srvAreaDB, dbNumber);
    if (Result == 0) {
      Srv_UnlockArea(server, srvAreaDB, dbNumber);
        return true;
    }
    return false;
}

int main() {
    std::cout << "========================================" << std::endl;
	std::cout << "S7 Server ISO-on-TCP (Snap7)" << std::endl;
    std::cout << "For Node-RED Testing" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Set up signal handler for Ctrl+C
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);

    // Create server instance
    S7Server = Srv_Create();
    if (!S7Server) {
        std::cerr << "ERROR: Failed to create server instance!" << std::endl;
        return 1;
    }

    // Configure server port (optional: use non-privileged port for testing)
    // Uncomment the following lines to use port 10102 instead of 102 (no admin required)
    // int customPort = 10102;
    // Srv_SetParam(S7Server, p_u16_LocalPort, &customPort);
    // std::cout << "NOTE: Using custom port 10102 (no admin privileges required)" << std::endl;

    // Configure PDU size (default is 480 bytes)
    // Increase to 960 bytes to allow more variables in MultiRead operations
    // Note: Client and server negotiate the PDU size (minimum of both)
    int pduSize = 960;
    Srv_SetParam(S7Server, p_i32_PDURequest, &pduSize);
    std::cout << "Server PDU size configured: " << pduSize << " bytes" << std::endl;
    std::cout << "NOTE: Larger PDU allows more variables per MultiRead/MultiWrite" << std::endl;

    // Load CSV configuration
    std::cout << "Loading CSV configuration from 'address.csv'..." << std::endl;
    std::vector<CSVConfigEntry> csvConfig = LoadCSVConfig("address.csv");
    
    // Create and initialize Data Blocks from CSV
    std::vector<DataBlock> dataBlocks;
    if (!csvConfig.empty()) {
        std::cout << "\nInitializing Data Blocks from CSV configuration..." << std::endl;
        dataBlocks = CreateDataBlocksFromCSV(csvConfig);
    } else {
        std::cerr << "WARNING: No CSV configuration loaded. Server will start with minimal configuration." << std::endl;
    }
    
    // Allocate standard memory areas for PLC simulation
    // Inputs (256 bytes)
    byte* IArea = new byte[256]();
    
    // Outputs (256 bytes)
    byte* QArea = new byte[256]();
    
    // Flags/Merkers (256 bytes)
    byte* MArea = new byte[256]();
    
	// Timers (512 bytes)
    byte* TArea = new byte[512]();
    
    // Counters (512 bytes)
    byte* CArea = new byte[512]();
  
    std::cout << "Initializing memory areas..." << std::endl;

    // Register memory areas with the server
	int Result;
    bool registrationFailed = false;
    
    // Register dynamically created Data Blocks
    for (const auto& db : dataBlocks) 
    {
        Result = Srv_RegisterArea(S7Server, srvAreaDB, db.number, db.data, db.size);
		if (Result != 0) 
        {
			char ErrorText[256];
			Srv_ErrorText(Result, ErrorText, 256);
			std::cerr << "ERROR: Failed to register DB" << db.number << "! Error: " << ErrorText << std::endl;
			registrationFailed = true;
			break;
        }
  		else 
        {
			std::cout << "  Registered DB" << db.number << " (" << db.size << " bytes) at address " << static_cast<void*>(db.data) << std::endl;

        }
    }


	//check Data block accessibility
    for (const auto& db : dataBlocks)
    {
        if (!VerifyDBAreaAccessible(S7Server, db.number, db.size)) {
            std::cerr << "ERROR: DB" << db.number << " not accessible!" << std::endl;
        }
        else {
            std::cout << "  ? DB" << db.number << " verified accessible" << std::endl;
        }
    }

    if (!registrationFailed) {
        Result = Srv_RegisterArea(S7Server, srvAreaPE, 0, IArea, 256);
        if (Result != 0) {
			std::cerr << "ERROR: Failed to register Input area!" << std::endl;
			registrationFailed = true;
        }
    }
    
    if (!registrationFailed) {
		Result = Srv_RegisterArea(S7Server, srvAreaPA, 0, QArea, 256);
        if (Result != 0) {
			std::cerr << "ERROR: Failed to register Output area!" << std::endl;
            registrationFailed = true;
        }
    }
    
    if (!registrationFailed) {
		Result = Srv_RegisterArea(S7Server, srvAreaMK, 0, MArea, 256);
        if (Result != 0) {
			std::cerr << "ERROR: Failed to register Flags area!" << std::endl;
			registrationFailed = true;
        }
    }
    
    if (!registrationFailed) {
        Result = Srv_RegisterArea(S7Server, srvAreaTM, 0, TArea, 512);
        if (Result != 0) {
            std::cerr << "ERROR: Failed to register Timers area!" << std::endl;
			registrationFailed = true;
        }
    }
    
    if (!registrationFailed) {
		Result = Srv_RegisterArea(S7Server, srvAreaCT, 0, CArea, 512);
        if (Result != 0) {
			std::cerr << "ERROR: Failed to register Counters area!" << std::endl;
            registrationFailed = true;
        }
}

    if (registrationFailed) {
        // Cleanup on failure
		Srv_Destroy(&S7Server);
		CleanupResources(dataBlocks, IArea, QArea, MArea, TArea, CArea);
		return 1;
    }

    std::cout << "Memory areas registered successfully." << std::endl;

    // Set event callbacks
    Srv_SetEventsCallback(S7Server, EventCallback, nullptr);
    Srv_SetReadEventsCallback(S7Server, ReadEventCallback, nullptr);
    
    // IMPORTANT: RWAreaCallback is intentionally NOT registered here.
    // When a RWAreaCallback is registered, Snap7 delegates ALL read/write operations
    // to it, expecting the callback to handle data transfer. However, the current
    // RWAreaCallback implementation only logs writes and doesn't actually copy data,
    // causing all read/write operations to fail silently.
    // 
    // By NOT registering the callback, Snap7 automatically handles all read/write
    // operations using its internal buffers with the registered memory areas,
    // allowing clients to successfully read and write data.
    // 
    // If you need custom read/write logic or logging in the future, implement
    // complete data transfer in RWAreaCallback before re-enabling this line:
    // Srv_SetRWAreaCallback(S7Server, RWAreaCallback, nullptr);

    // Set event mask to capture important events
    Srv_SetMask(S7Server, mkEvent, 0xFFFFFFFF);
    Srv_SetMask(S7Server, mkLog, 0x00000000); // Disable excessive logging

    // Get the configured port
    int serverPort = 102;
    Srv_GetParam(S7Server, p_u16_LocalPort, &serverPort);
  
    // Start the server
    std::cout << "Starting server on port " << serverPort << "..." << std::endl;
    if (serverPort == 102) {
        std::cout << "NOTE: Port 102 requires administrator privileges!" << std::endl;
    }
    Result = Srv_Start(S7Server);
    
    if (Result != 0) {
        char ErrorText[256];
        Srv_ErrorText(Result, ErrorText, 256);
        std::cerr << "ERROR: Failed to start server: " << ErrorText << std::endl;
        std::cerr << "Note: Port 102 requires administrator privileges on Windows." << std::endl;
      std::cerr << "      Run this application as Administrator." << std::endl;
        
        // Cleanup
    Srv_Destroy(&S7Server);
		CleanupResources(dataBlocks, IArea, QArea, MArea, TArea, CArea);
		return 1;
    }

	std::cout << "\n*** Server started successfully! ***\n" << std::endl;
	DisplayConfig(dataBlocks);
    
    // Initialize tag states for dynamic value updates
    std::vector<TagState> tagStates;
    if (!csvConfig.empty()) {
        tagStates = InitializeTagStates(csvConfig, dataBlocks);
        std::cout << "Dynamic tag value updates enabled with 100ms update interval." << std::endl;
    }
    
    std::cout << "Server is running. Press Ctrl+C to stop.\n" << std::endl;

    // Main server loop with time-based status updates and tag value updates
    auto lastStatusTime = std::chrono::steady_clock::now();
    const auto statusInterval = std::chrono::seconds(30);
    
    while (ServerRunning) {
        // Update tag values every 100ms
        if (!tagStates.empty()) {
            UpdateTagValues(tagStates);
        }
        
        // Display status every 30 seconds
		auto currentTime = std::chrono::steady_clock::now();
		if (currentTime - lastStatusTime >= statusInterval) {
		DisplayStatus(S7Server);
		    lastStatusTime = currentTime;
		}
        
        // Sleep for 100ms (threshold as specified in requirements)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Shutdown
    std::cout << "\nStopping server..." << std::endl;
	Srv_Stop(S7Server);
    
    std::cout << "Cleaning up resources..." << std::endl;
    Srv_Destroy(&S7Server);
    
    // Free allocated memory
    CleanupResources(dataBlocks, IArea, QArea, MArea, TArea, CArea);

	std::cout << "Server stopped successfully. Goodbye!" << std::endl;
    return 0;
}
