

#include "../include/DataStorage.hpp"
#include <iostream>
#include <sstream>

namespace Telemetry {

DataStorage::DataStorage(size_t maxRamCapacity, const std::string& logFilePath)
    : m_maxRamCapacity(maxRamCapacity), m_logFilePath(logFilePath) {
    m_fileStream.open(m_logFilePath, std::ios::out | std::ios::app);
    if (!m_fileStream.is_open()) {
        std::cerr << "[DataStorage] Error opening disk log file: " << m_logFilePath << std::endl;
    }
}

DataStorage::~DataStorage() {
    flushToDisk();
    if (m_fileStream.is_open()) m_fileStream.close();
}

void DataStorage::storeBlock(const StorageBlock& block) {
    std::unique_lock<std::shared_mutex> lock(m_rwMutex);
    
    if (m_ramBuffer.size() >= m_maxRamCapacity) {
        // Demote oldest RAM block to disk storage (Cold Tier)
        StorageBlock demoted = m_ramBuffer.front();
        m_ramBuffer.pop_front();
        demoted.tier = DataTier::COLD_DISK;

        if (m_fileStream.is_open()) {
            std::ostringstream oss;
            oss << "[" << demoted.timestamp << "] "
                << "BlockID:" << demoted.blockId << " "
                << "Value:" << demoted.metricValue << " "
                << "Tier:" << demoted.getTierString() << " "
                << "Severity:" << demoted.getSeverityString() << "\n";
            std::string logLine = oss.str();
            m_fileStream << logLine;
            m_diskBytesWritten += logLine.size();
        }
    }

    m_ramBuffer.push_back(block);
}

std::vector<StorageBlock> DataStorage::getRecentBlocks(size_t count) {
    std::shared_lock<std::shared_mutex> lock(m_rwMutex);
    std::vector<StorageBlock> result;

    size_t available = m_ramBuffer.size();
    size_t fetchCount = (count < available) ? count : available;

    auto it = m_ramBuffer.rbegin();
    for (size_t i = 0; i < fetchCount; ++i, ++it) {
        result.push_back(*it);
    }

    return result;
}

uint64_t DataStorage::flushToDisk() {
    std::unique_lock<std::shared_mutex> lock(m_rwMutex);
    uint64_t flushedCount = 0;

    if (m_fileStream.is_open()) {
        while (!m_ramBuffer.empty()) {
            StorageBlock block = m_ramBuffer.front();
            m_ramBuffer.pop_front();
            block.tier = DataTier::COLD_DISK;

            std::ostringstream oss;
            oss << "[" << block.timestamp << "] "
                << "BlockID:" << block.blockId << " "
                << "Value:" << block.metricValue << " "
                << "Tier:" << block.getTierString() << " "
                << "Severity:" << block.getSeverityString() << "\n";
            std::string logLine = oss.str();
            m_fileStream << logLine;
            m_diskBytesWritten += logLine.size();
            flushedCount++;
        }
        m_fileStream.flush();
    }

    return flushedCount;
}

uint64_t DataStorage::getDiskBytesWritten() const {
    std::shared_lock<std::shared_mutex> lock(m_rwMutex);
    return m_diskBytesWritten;
}

} // namespace Telemetry
