#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <cstdint>

/*
 * DataBlock - Structure holding Data Block information
 */
struct DataBlockInfo {
    int number;
    int size;
    uint8_t* data;
};

/*
 * MemoryAccessor - Thread-safe accessor for S7 memory areas
 * 
 * Provides thread-safe read access to Data Blocks and other memory areas.
 * Uses mutex protection to prevent race conditions when HTTP server reads
 * memory while S7 server or dynamic update thread modifies it.
 */
class MemoryAccessor {
public:
    MemoryAccessor();
    ~MemoryAccessor();

    // Register memory areas (called once at startup)
    void registerDataBlock(int number, uint8_t* data, int size);
    void registerInputArea(uint8_t* data, int size);
    void registerOutputArea(uint8_t* data, int size);
    void registerFlagArea(uint8_t* data, int size);
    void registerTimerArea(uint8_t* data, int size);
    void registerCounterArea(uint8_t* data, int size);

    // Thread-safe read operations (returns copy of data)
    std::vector<uint8_t> readDataBlock(int number) const;
    std::vector<uint8_t> readInputArea() const;
    std::vector<uint8_t> readOutputArea() const;
    std::vector<uint8_t> readFlagArea() const;
    std::vector<uint8_t> readTimerArea() const;
    std::vector<uint8_t> readCounterArea() const;

    // Get metadata
    std::vector<int> getDataBlockNumbers() const;
    int getDataBlockSize(int number) const;
    int getInputAreaSize() const;
    int getOutputAreaSize() const;
    int getFlagAreaSize() const;
    int getTimerAreaSize() const;
    int getCounterAreaSize() const;

    // Direct access with manual locking (for batch operations)
    std::unique_lock<std::mutex> acquireLock();
    const uint8_t* getDataBlockPointer(int number) const;

private:
    mutable std::mutex m_mutex;

    std::map<int, DataBlockInfo> m_dataBlocks;
    
    uint8_t* m_inputArea;
    int m_inputAreaSize;
    
    uint8_t* m_outputArea;
    int m_outputAreaSize;
    
    uint8_t* m_flagArea;
    int m_flagAreaSize;
    
    uint8_t* m_timerArea;
    int m_timerAreaSize;
    
    uint8_t* m_counterArea;
    int m_counterAreaSize;

    // Helper to copy memory safely
    std::vector<uint8_t> copyMemory(const uint8_t* src, int size) const;
};
