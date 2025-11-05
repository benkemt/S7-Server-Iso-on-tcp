/*
 * S7 Server ISO-on-TCP Implementation
 * Using Snap7 Library for Node-RED Testing
 * 
 * This server creates a Siemens S7 compatible server using ISO-on-TCP protocol
 * that can be used for testing Node-RED S7 applications.
 */

#include <iostream>
#include <cstring>
#include <csignal>
#include <thread>
#include <chrono>
#include "snap7.h"

// Global server instance
TS7Server* S7Server = nullptr;
bool ServerRunning = true;

// Signal handler for graceful shutdown
void SignalHandler(int signal) {
    std::cout << "\nShutdown signal received. Stopping server..." << std::endl;
    ServerRunning = false;
}

// Event callback function
void S7API EventCallback(void* usrPtr, PSrvEvent PEvent, int Size) {
    // Log server events
    std::string EventText;
    
    switch (PEvent->EvtCode) {
        case evcServerStarted:
            EventText = "Server started";
            break;
        case evcServerStopped:
            EventText = "Server stopped";
            break;
        case evcClientConnected:
            EventText = "Client connected";
            break;
        case evcClientDisconnected:
            EventText = "Client disconnected";
            break;
        case evcPDUincoming:
            EventText = "PDU incoming";
            break;
        default:
            EventText = "Unknown event";
            break;
    }
    
    std::cout << "[EVENT] " << EventText << " (Code: " << PEvent->EvtCode << ")" << std::endl;
}

// Read event callback
void S7API ReadEventCallback(void* usrPtr, PSrvEvent PEvent, int Size) {
    std::cout << "[READ] Area: " << PEvent->EvtParam1 
              << ", Start: " << PEvent->EvtParam2 
              << ", Size: " << PEvent->EvtParam3 << std::endl;
}

// Write event callback
void S7API WriteEventCallback(void* usrPtr, PSrvEvent PEvent, int Size) {
    std::cout << "[WRITE] Area: " << PEvent->EvtParam1 
              << ", Start: " << PEvent->EvtParam2 
              << ", Size: " << PEvent->EvtParam3 << std::endl;
}

// Display server configuration
void DisplayConfig() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "S7 Server Configuration:" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Protocol: ISO-on-TCP" << std::endl;
    std::cout << "Port: 102" << std::endl;
    std::cout << "Data Blocks:" << std::endl;
    std::cout << "  - DB1: 256 bytes (General purpose)" << std::endl;
    std::cout << "  - DB2: 512 bytes (Extended data)" << std::endl;
    std::cout << "  - DB3: 128 bytes (Test data)" << std::endl;
    std::cout << "Inputs (I):  256 bytes" << std::endl;
    std::cout << "Outputs (Q): 256 bytes" << std::endl;
    std::cout << "Flags (M):   256 bytes" << std::endl;
    std::cout << "Timers (T):  512 bytes" << std::endl;
    std::cout << "Counters (C): 512 bytes" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// Display server status
void DisplayStatus(TS7Server* Server) {
    int Status = Srv_GetStatus(Server);
    int ClientsCount = Srv_GetParam(Server, p_ServerClientsCount);
    
    std::cout << "Server Status: " << (Status == SrvRunning ? "RUNNING" : "STOPPED") << std::endl;
    std::cout << "Connected Clients: " << ClientsCount << std::endl;
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

    // Allocate memory areas for PLC simulation
    // DB1 - Data Block 1 (256 bytes)
    byte* DB1 = new byte[256]();
    
    // DB2 - Data Block 2 (512 bytes)
    byte* DB2 = new byte[512]();
    
    // DB3 - Data Block 3 (128 bytes)
    byte* DB3 = new byte[128]();
    
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

    // Initialize some test data in DB1
    DB1[0] = 42;      // Test value at DB1.DBB0
    DB1[1] = 100;     // Test value at DB1.DBB1
    DB1[2] = 0xFF;    // Test value at DB1.DBB2
    
    // Initialize some test data in DB2
    DB2[0] = 1;       // Test value at DB2.DBB0
    DB2[1] = 2;       // Test value at DB2.DBB1
    DB2[2] = 3;       // Test value at DB2.DBB2
    
    std::cout << "Initializing memory areas..." << std::endl;

    // Register memory areas with the server
    int Result;
    
    Result = Srv_RegisterArea(S7Server, srvAreaDB, 1, DB1, 256);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register DB1!" << std::endl;
        return 1;
    }
    
    Result = Srv_RegisterArea(S7Server, srvAreaDB, 2, DB2, 512);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register DB2!" << std::endl;
        return 1;
    }
    
    Result = Srv_RegisterArea(S7Server, srvAreaDB, 3, DB3, 128);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register DB3!" << std::endl;
        return 1;
    }
    
    Result = Srv_RegisterArea(S7Server, srvAreaPE, 0, IArea, 256);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register Input area!" << std::endl;
        return 1;
    }
    
    Result = Srv_RegisterArea(S7Server, srvAreaPA, 0, QArea, 256);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register Output area!" << std::endl;
        return 1;
    }
    
    Result = Srv_RegisterArea(S7Server, srvAreaMK, 0, MArea, 256);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register Flags area!" << std::endl;
        return 1;
    }
    
    Result = Srv_RegisterArea(S7Server, srvAreaTM, 0, TArea, 512);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register Timers area!" << std::endl;
        return 1;
    }
    
    Result = Srv_RegisterArea(S7Server, srvAreaCT, 0, CArea, 512);
    if (Result != 0) {
        std::cerr << "ERROR: Failed to register Counters area!" << std::endl;
        return 1;
    }

    std::cout << "Memory areas registered successfully." << std::endl;

    // Set event callbacks
    Srv_SetEventsCallback(S7Server, EventCallback, nullptr);
    Srv_SetReadEventsCallback(S7Server, ReadEventCallback, nullptr);
    Srv_SetWriteEventsCallback(S7Server, WriteEventCallback, nullptr);

    // Set event mask to capture important events
    Srv_SetMask(S7Server, mkEvent, 0xFFFFFFFF);
    Srv_SetMask(S7Server, mkLog, 0x00000000); // Disable excessive logging

    // Start the server on default port 102
    std::cout << "Starting server on port 102..." << std::endl;
    Result = Srv_Start(S7Server);
    
    if (Result != 0) {
        char ErrorText[256];
        Srv_ErrorText(Result, ErrorText, 256);
        std::cerr << "ERROR: Failed to start server: " << ErrorText << std::endl;
        std::cerr << "Note: Port 102 requires administrator privileges on Windows." << std::endl;
        std::cerr << "      Run this application as Administrator." << std::endl;
        
        // Cleanup
        Srv_Destroy(&S7Server);
        delete[] DB1;
        delete[] DB2;
        delete[] DB3;
        delete[] IArea;
        delete[] QArea;
        delete[] MArea;
        delete[] TArea;
        delete[] CArea;
        return 1;
    }

    std::cout << "\n*** Server started successfully! ***\n" << std::endl;
    DisplayConfig();
    
    std::cout << "Server is running. Press Ctrl+C to stop.\n" << std::endl;

    // Main server loop with time-based status updates
    auto lastStatusTime = std::chrono::steady_clock::now();
    const auto statusInterval = std::chrono::seconds(30);
    
    while (ServerRunning) {
        // Display status every 30 seconds
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - lastStatusTime >= statusInterval) {
            DisplayStatus(S7Server);
            lastStatusTime = currentTime;
        }
        
        // Sleep for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shutdown
    std::cout << "\nStopping server..." << std::endl;
    Srv_Stop(S7Server);
    
    std::cout << "Cleaning up resources..." << std::endl;
    Srv_Destroy(&S7Server);
    
    // Free allocated memory
    delete[] DB1;
    delete[] DB2;
    delete[] DB3;
    delete[] IArea;
    delete[] QArea;
    delete[] MArea;
    delete[] TArea;
    delete[] CArea;

    std::cout << "Server stopped successfully. Goodbye!" << std::endl;
    return 0;
}
