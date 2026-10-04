

#ifndef SENSOR_MANAGER_HPP
#define SENSOR_MANAGER_HPP

#include "Common.hpp"
#include <random>
#include <mutex>

namespace Telemetry {

class SensorManager {
public:
    SensorManager();
    ~SensorManager() = default;

    StorageBlock generateBlock(uint32_t blockId);
    std::vector<StorageBlock> generateBlockBatch();
    void setFaultInjection(bool enable) { m_faultInjectionActive = enable; }

private:
    std::mt19937 m_rng;
    std::mutex m_mutex;
    bool m_faultInjectionActive{false};
};

} // namespace Telemetry

#endif // SENSOR_MANAGER_HPP
