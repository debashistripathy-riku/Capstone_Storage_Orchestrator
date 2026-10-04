

#ifndef DATA_STORAGE_HPP
#define DATA_STORAGE_HPP

#include "Common.hpp"
#include <vector>
#include <deque>
#include <shared_mutex>
#include <fstream>

namespace Telemetry {

class DataStorage {
public:
    explicit DataStorage(size_t maxRamCapacity = 100, const std::string& logFilePath = "virtual_storage.log");
    ~DataStorage();

    void storeBlock(const StorageBlock& block);
    std::vector<StorageBlock> getRecentBlocks(size_t count);
    uint64_t flushToDisk();
    uint64_t getDiskBytesWritten() const;

private:
    size_t m_maxRamCapacity;
    std::string m_logFilePath;
    std::deque<StorageBlock> m_ramBuffer;
    mutable std::shared_mutex m_rwMutex;
    uint64_t m_diskBytesWritten{0};
    std::ofstream m_fileStream;
};

} // namespace Telemetry

#endif // DATA_STORAGE_HPP
