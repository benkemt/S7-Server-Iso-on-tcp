/*
 * S7 Client Test Application
 * Using Snap7 Library to test connection and variable limits
 * 
 * This client tests reading multiple variables from the S7 Server
 * to verify if there's a 20 variable limit when using Snap7.
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <iomanip>
#include "../S7Server/snap7/snap7.h"

// Constants
const int REAL_SIZE = 4;  // S7 REAL data type size in bytes

// Structure to hold a variable read request
struct S7Variable {
    int dbNumber;
    int offset;
    float value;
    bool readSuccess;
};

// Helper function to convert S7 REAL format (big-endian IEEE 754) to float
float GetReal(byte* buffer, int offset) {
    // S7 uses big-endian byte order, convert to little-endian (Windows x86/x64)
    byte floatBytes[4];
    floatBytes[0] = buffer[offset + 3];  // Least significant byte
 floatBytes[1] = buffer[offset + 2];
    floatBytes[2] = buffer[offset + 1];
    floatBytes[3] = buffer[offset + 0];  // Most significant byte
    
    return *reinterpret_cast<float*>(floatBytes);
}

// Test reading a single variable using Cli_ReadArea
bool ReadSingleVariable(S7Object client, S7Variable& var) {
    byte buffer[REAL_SIZE];
    
    int result = Cli_ReadArea(client, S7AreaDB, var.dbNumber, var.offset, REAL_SIZE, S7WLByte, buffer);
    
    if (result == 0) {
        var.value = GetReal(buffer, 0);
  var.readSuccess = true;
        return true;
    } else {
        var.readSuccess = false;
        return false;
    }
}

// Test reading multiple variables using Cli_ReadMultiVars
bool ReadMultipleVariables(S7Object client, std::vector<S7Variable>& variables) {
    int varCount = variables.size();
    
    // Allocate arrays for MultiRead
    TS7DataItem* items = new TS7DataItem[varCount];
    byte** buffers = new byte*[varCount];
    
    // Initialize read items
    for (int i = 0; i < varCount; i++) {
        buffers[i] = new byte[REAL_SIZE];
     
        items[i].Area = S7AreaDB;
        items[i].WordLen = S7WLByte;
     items[i].DBNumber = variables[i].dbNumber;
        items[i].Start = variables[i].offset;
        items[i].Amount = REAL_SIZE;
        items[i].pdata = buffers[i];
    }
    
    // Perform multi-read
    int result = Cli_ReadMultiVars(client, items, varCount);
    
    // Process results
    bool allSuccess = true;
    for (int i = 0; i < varCount; i++) {
        if (items[i].Result == 0) {
            variables[i].value = GetReal(buffers[i], 0);
  variables[i].readSuccess = true;
        } else {
variables[i].readSuccess = false;
            allSuccess = false;
        }
    }
    
    // Cleanup
    for (int i = 0; i < varCount; i++) {
 delete[] buffers[i];
    }
  delete[] buffers;
    delete[] items;
    
    return allSuccess;
}

// Display connection info
void DisplayConnectionInfo(S7Object client) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Connection Information:" << std::endl;
    std::cout << "========================================" << std::endl;
    
    int pduRequested, pduNegotiated;
    Cli_GetParam(client, p_u16_RemotePort, &pduRequested);
    Cli_GetPduLength(client, &pduRequested, &pduNegotiated);
  
    std::cout << "PDU Negotiated: " << pduNegotiated << " bytes" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "S7 Client Test Application (Snap7)" << std::endl;
    std::cout << "Testing Variable Read Limits" << std::endl;
  std::cout << "========================================\n" << std::endl;

    // Parse command line arguments
    std::string serverIP = "127.0.0.1";
    int rack = 0;
    int slot = 0;
    int port = 102;
    
    if (argc >= 2) {
        serverIP = argv[1];
    }
    if (argc >= 3) {
    rack = std::stoi(argv[2]);
    }
    if (argc >= 4) {
        slot = std::stoi(argv[3]);
    }
    if (argc >= 5) {
        port = std::stoi(argv[4]);
    }
    
    std::cout << "Target Server: " << serverIP << std::endl;
    std::cout << "Rack: " << rack << ", Slot: " << slot << std::endl;
    std::cout << "Port: " << port << "\n" << std::endl;

    // Create client instance
    S7Object client = Cli_Create();
    if (!client) {
  std::cerr << "ERROR: Failed to create client instance!" << std::endl;
        return 1;
    }

    // Set connection parameters if using non-standard port
    if (port != 102) {
        Cli_SetParam(client, p_u16_RemotePort, &port);
    }

    // Request larger PDU size to allow more variables
    // Default is 480 bytes, let's try 960 bytes
    int requestedPDU = 960;
    Cli_SetParam(client, p_i32_PDURequest, &requestedPDU);
    std::cout << "Requesting PDU size: " << requestedPDU << " bytes\n" << std::endl;

    // Connect to server
    std::cout << "Connecting to S7 server..." << std::endl;
    int result = Cli_ConnectTo(client, serverIP.c_str(), rack, slot);
    
    if (result != 0) {
        char errorText[256];
        Cli_ErrorText(result, errorText, 256);
        std::cerr << "ERROR: Connection failed: " << errorText << std::endl;
   Cli_Destroy(&client);
        return 1;
    }
 
    std::cout << "Connected successfully!\n" << std::endl;
    DisplayConnectionInfo(client);

    // Test 1: Read variables individually (to establish baseline)
    std::cout << "========================================" << std::endl;
    std::cout << "Test 1: Reading 5 variables individually" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<S7Variable> testVars1 = {
  {101, 0, 0.0f, false},
      {101, 4, 0.0f, false},
        {101, 8, 0.0f, false},
        {101, 12, 0.0f, false},
        {101, 16, 0.0f, false}
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& var : testVars1) {
        ReadSingleVariable(client, var);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    for (size_t i = 0; i < testVars1.size(); i++) {
        std::cout << "  DB" << testVars1[i].dbNumber << ".REAL" << testVars1[i].offset << ": "
   << std::fixed << std::setprecision(2) << testVars1[i].value
 << (testVars1[i].readSuccess ? " [OK]" : " [FAIL]") << std::endl;
    }
    std::cout << "Time: " << duration.count() << " ms\n" << std::endl;

 // Test 2: Read 20 variables using MultiRead
    std::cout << "========================================" << std::endl;
std::cout << "Test 2: Reading 20 variables (MultiRead)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<S7Variable> testVars20;
    for (int i = 0; i < 20; i++) {
        testVars20.push_back({101, i * 4, 0.0f, false});
    }
  
    start = std::chrono::high_resolution_clock::now();
    bool success20 = ReadMultipleVariables(client, testVars20);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    int successCount20 = 0;
    for (size_t i = 0; i < testVars20.size(); i++) {
   if (testVars20[i].readSuccess) successCount20++;
   std::cout << "  DB" << testVars20[i].dbNumber << ".REAL" << testVars20[i].offset << ": "
     << std::fixed << std::setprecision(2) << testVars20[i].value
         << (testVars20[i].readSuccess ? " [OK]" : " [FAIL]") << std::endl;
    }
    std::cout << "Success: " << successCount20 << "/20 variables" << std::endl;
    std::cout << "Time: " << duration.count() << " ms\n" << std::endl;

    // Test 3: Read 30 variables using MultiRead (testing beyond 20 limit)
 std::cout << "========================================" << std::endl;
    std::cout << "Test 3: Reading 30 variables (MultiRead)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<S7Variable> testVars30;
 for (int i = 0; i < 30; i++) {
        testVars30.push_back({101, i * 4, 0.0f, false});
    }
    
    start = std::chrono::high_resolution_clock::now();
    bool success30 = ReadMultipleVariables(client, testVars30);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    int successCount30 = 0;
    for (size_t i = 0; i < testVars30.size(); i++) {
    if (testVars30[i].readSuccess) successCount30++;
        std::cout << "DB" << testVars30[i].dbNumber << ".REAL" << testVars30[i].offset << ": "
           << std::fixed << std::setprecision(2) << testVars30[i].value
  << (testVars30[i].readSuccess ? " [OK]" : " [FAIL]") << std::endl;
    }
    std::cout << "Success: " << successCount30 << "/30 variables" << std::endl;
    std::cout << "Time: " << duration.count() << " ms\n" << std::endl;

  // Test 4: Read 50 variables using MultiRead (stress test)
    std::cout << "========================================" << std::endl;
    std::cout << "Test 4: Reading 50 variables (MultiRead)" << std::endl;
  std::cout << "========================================" << std::endl;
 
    std::vector<S7Variable> testVars50;
    for (int i = 0; i < 50; i++) {
        testVars50.push_back({101, i * 4, 0.0f, false});
    }
    
    start = std::chrono::high_resolution_clock::now();
    bool success50 = ReadMultipleVariables(client, testVars50);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    int successCount50 = 0;
    for (size_t i = 0; i < testVars50.size(); i++) {
        if (testVars50[i].readSuccess) successCount50++;
    }
    std::cout << "Success: " << successCount50 << "/50 variables" << std::endl;
    std::cout << "Time: " << duration.count() << " ms\n" << std::endl;

    // Test 5: Workaround - Read as contiguous block (bypasses 20-variable limit)
    std::cout << "========================================" << std::endl;
    std::cout << "Test 5: Reading 50 REALs as contiguous block" << std::endl;
    std::cout << "========================================" << std::endl;
    
    byte blockBuffer[200];  // 50 REALs × 4 bytes
    start = std::chrono::high_resolution_clock::now();
    int blockResult = Cli_ReadArea(client, S7AreaDB, 101, 0, 200, S7WLByte, blockBuffer);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (blockResult == 0) {
        std::cout << "? Successfully read 200 bytes (50 REALs) in one operation!" << std::endl;
        std::cout << "Sample values:" << std::endl;
        for (int i = 0; i < 5; i++) {
          float value = GetReal(blockBuffer, i * 4);
            std::cout << "  DB101.REAL" << (i * 4) << ": " << std::fixed << std::setprecision(2) << value << std::endl;
   }
        std::cout << "  ... (45 more values)" << std::endl;
      std::cout << "Time: " << duration.count() << " ms" << std::endl;
    } else {
        char errorText[256];
        Cli_ErrorText(blockResult, errorText, 256);
        std::cout << "? Block read failed: " << errorText << std::endl;
    }
    std::cout << std::endl;

    // Test 6: Workaround - Batch reading (3 batches of 20, then 10)
    std::cout << "========================================" << std::endl;
    std::cout << "Test 6: Reading 50 variables in batches" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::vector<S7Variable> testVars50Batched;
 for (int i = 0; i < 50; i++) {
        testVars50Batched.push_back({101, i * 4, 0.0f, false});
    }
    
    int batchSuccessCount = 0;
    start = std::chrono::high_resolution_clock::now();
    
    // Read in batches of 20
    for (int batch = 0; batch < 3; batch++) {
        int batchStart = batch * 20;
        int batchSize = (batch == 2) ? 10 : 20;  // Last batch is only 10
        
   std::vector<S7Variable> batchVars(testVars50Batched.begin() + batchStart,
              testVars50Batched.begin() + batchStart + batchSize);
        
 bool batchSuccess = ReadMultipleVariables(client, batchVars);

        // Copy results back
   for (int i = 0; i < batchSize; i++) {
 testVars50Batched[batchStart + i] = batchVars[i];
            if (batchVars[i].readSuccess) batchSuccessCount++;
        }
    
    std::cout << "Batch " << (batch + 1) << " (" << batchSize << " vars): "
   << (batchSuccess ? "? SUCCESS" : "? FAILED") << std::endl;
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Total success: " << batchSuccessCount << "/50 variables" << std::endl;
    std::cout << "Time: " << duration.count() << " ms" << std::endl;
  std::cout << std::endl;

    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test Summary:" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "20 variables: " << successCount20 << "/20 " << (success20 ? "[PASS]" : "[FAIL]") << std::endl;
  std::cout << "30 variables: " << successCount30 << "/30 " << (success30 ? "[PASS]" : "[FAIL]") << std::endl;
    std::cout << "50 variables: " << successCount50 << "/50 " << (success50 ? "[PASS]" : "[FAIL]") << std::endl;
    std::cout << "Contiguous block (50 REALs): " << (blockResult == 0 ? "? [PASS]" : "? [FAIL]") << std::endl;
    std::cout << "Batched read (50 vars): " << batchSuccessCount << "/50 " << (batchSuccessCount == 50 ? "[PASS]" : "[FAIL]") << std::endl;
    std::cout << "========================================" << std::endl;
    
    if (successCount20 == 20 && successCount30 < 30) {
        std::cout << "\nCONCLUSION: 20 variable limit confirmed in Snap7!" << std::endl;
        std::cout << "This is a hard-coded limit (MaxVars = 20) in snap7.h" << std::endl;
        std::cout << "\nWORKAROUNDS DEMONSTRATED:" << std::endl;
        std::cout << "  ? Contiguous block read: " << (blockResult == 0 ? "SUCCESS" : "FAILED") << std::endl;
    std::cout << "  ? Batched reading: " << (batchSuccessCount == 50 ? "SUCCESS" : "FAILED") << std::endl;
    } else if (successCount50 == 50) {
        std::cout << "\nCONCLUSION: No 20 variable limit detected in Snap7!" << std::endl;
    } else {
        std::cout << "\nCONCLUSION: Results are inconclusive. Check server configuration." << std::endl;
    }
    std::cout << "========================================\n" << std::endl;

    // Disconnect and cleanup
    std::cout << "Disconnecting from server..." << std::endl;
    Cli_Disconnect(client);
    Cli_Destroy(&client);
 
    std::cout << "Client stopped successfully. Goodbye!" << std::endl;
    return 0;
}
