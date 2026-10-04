

#include "../include/SensorManager.hpp"
#include <chrono>

namespace Telemetry {

SensorManager::SensorManager() : m_rng(std::random_device{}()) {}

StorageBlock SensorManager::generateBlock(uint32_t blockId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    StorageBlock block;
    block.blockId = blockId;
    block.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    block.tier = DataTier::HOT_RAM;

    std::uniform_real_distribution<double> dist(10.0, 100.0);
    block.metricValue = dist(m_rng);

    if (m_faultInjectionActive) {
        block.metricValue += 150.0;
    }

    if (block.metricValue > 120.0) block.severity = SeverityLevel::CRITICAL;
    else if (block.metricValue > 80.0) block.severity = SeverityLevel::WARNING;
    else block.severity = SeverityLevel::NORMAL;

    return block;
}

std::vector<StorageBlock> SensorManager::generateBlockBatch() {
    return {
        generateBlock(101),
        generateBlock(102),
        generateBlock(103),
        generateBlock(104)
    };
}

} // namespace Telemetry
