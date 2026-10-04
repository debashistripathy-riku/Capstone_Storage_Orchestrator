/*
 * DriverInterface.hpp - Linux Device Driver & Software Fallback Interface (C++)
 * Student: Debashis Tripathy (Reg No: 2341020048)
 * Course: Wipro COE Capstone Project
 */

#ifndef DRIVER_INTERFACE_HPP
#define DRIVER_INTERFACE_HPP

#include "Common.hpp"
#include "../driver/telemetry_driver.h"
#include <mutex>
#include <queue>

namespace Telemetry {

class DriverInterface {
public:
    DriverInterface();
    ~DriverInterface();

    bool initialize();
    void closeDevice();

    bool readPacket(StorageBlock& block);
    bool writePacket(const StorageBlock& block);

    bool setSamplingRate(uint32_t samplingRateMs);
    bool triggerFlush();
    bool getDriverStats(driver_stats_t& stats);

    bool isKernelDriverLoaded() const { return m_isKernelDriverLoaded; }

private:
    int m_deviceFd{-1};
    bool m_isKernelDriverLoaded{false};

    // User-space Software Emulated Buffer (100% free software, $0 cost)
    std::queue<StorageBlock> m_mockBuffer;
    mutable std::mutex m_mockMutex;
    uint32_t m_mockSamplingRateMs{1000};
    uint64_t m_mockReads{0};
    uint64_t m_mockWrites{0};
};

} // namespace Telemetry

#endif // DRIVER_INTERFACE_HPP
