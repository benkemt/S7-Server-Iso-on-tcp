#include "MemoryAccessor.h"
#include <cstring>
#include <algorithm>

MemoryAccessor::MemoryAccessor()
    : m_inputArea(nullptr), m_inputAreaSize(0),
      m_outputArea(nullptr), m_outputAreaSize(0),
      m_flagArea(nullptr), m_flagAreaSize(0),
      m_timerArea(nullptr), m_timerAreaSize(0),
      m_counterArea(nullptr), m_counterAreaSize(0) {
}

MemoryAccessor::~MemoryAccessor() {
    // Note: We don't own the memory, just hold pointers
    // Actual cleanup is done by S7Server main()
}

void MemoryAccessor::registerDataBlock(int number, uint8_t* data, int size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    DataBlockInfo info;
    info.number = number;
    info.data = data;
    info.size = size;
    m_dataBlocks[number] = info;
}

void MemoryAccessor::registerInputArea(uint8_t* data, int size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_inputArea = data;
    m_inputAreaSize = size;
}

void MemoryAccessor::registerOutputArea(uint8_t* data, int size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_outputArea = data;
    m_outputAreaSize = size;
}

void MemoryAccessor::registerFlagArea(uint8_t* data, int size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_flagArea = data;
    m_flagAreaSize = size;
}

void MemoryAccessor::registerTimerArea(uint8_t* data, int size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_timerArea = data;
    m_timerAreaSize = size;
}

void MemoryAccessor::registerCounterArea(uint8_t* data, int size) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_counterArea = data;
    m_counterAreaSize = size;
}

std::vector<uint8_t> MemoryAccessor::readDataBlock(int number) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_dataBlocks.find(number);
    if (it != m_dataBlocks.end()) {
        return copyMemory(it->second.data, it->second.size);
    }
    return std::vector<uint8_t>();
}

std::vector<uint8_t> MemoryAccessor::readInputArea() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return copyMemory(m_inputArea, m_inputAreaSize);
}

std::vector<uint8_t> MemoryAccessor::readOutputArea() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return copyMemory(m_outputArea, m_outputAreaSize);
}

std::vector<uint8_t> MemoryAccessor::readFlagArea() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return copyMemory(m_flagArea, m_flagAreaSize);
}

std::vector<uint8_t> MemoryAccessor::readTimerArea() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return copyMemory(m_timerArea, m_timerAreaSize);
}

std::vector<uint8_t> MemoryAccessor::readCounterArea() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return copyMemory(m_counterArea, m_counterAreaSize);
}

std::vector<int> MemoryAccessor::getDataBlockNumbers() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<int> numbers;
    for (const auto& pair : m_dataBlocks) {
        numbers.push_back(pair.first);
    }
    std::sort(numbers.begin(), numbers.end());
    return numbers;
}

int MemoryAccessor::getDataBlockSize(int number) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_dataBlocks.find(number);
    return (it != m_dataBlocks.end()) ? it->second.size : 0;
}

int MemoryAccessor::getInputAreaSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_inputAreaSize;
}

int MemoryAccessor::getOutputAreaSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_outputAreaSize;
}

int MemoryAccessor::getFlagAreaSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_flagAreaSize;
}

int MemoryAccessor::getTimerAreaSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_timerAreaSize;
}

int MemoryAccessor::getCounterAreaSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_counterAreaSize;
}

std::unique_lock<std::mutex> MemoryAccessor::acquireLock() {
    return std::unique_lock<std::mutex>(m_mutex);
}

const uint8_t* MemoryAccessor::getDataBlockPointer(int number) const {
    // Note: Caller must hold lock before calling this
    auto it = m_dataBlocks.find(number);
    return (it != m_dataBlocks.end()) ? it->second.data : nullptr;
}

std::vector<uint8_t> MemoryAccessor::copyMemory(const uint8_t* src, int size) const {
    if (!src || size <= 0) {
        return std::vector<uint8_t>();
    }
    return std::vector<uint8_t>(src, src + size);
}
