
#include "../include/DriverInterface.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>

namespace Telemetry {

DriverInterface::DriverInterface() = default;

DriverInterface::~DriverInterface() {
    closeDevice();
}

bool DriverInterface::initialize() {
    m_deviceFd = open(DEVICE_PATH, O_RDWR);
    if (m_deviceFd >= 0) {
        m_isKernelDriverLoaded = true;
        std::cout << "[DriverInterface] Connected to Linux Kernel Virtual Device Driver: " << DEVICE_PATH << std::endl;
        return true;
    }

    m_isKernelDriverLoaded = false;
    std::cout << "[DriverInterface] Kernel driver (" << DEVICE_PATH 
              << ") not loaded. Running pure C++ Software Emulated Device Driver ($0 Cost)." << std::endl;
    return true;
}

void DriverInterface::closeDevice() {
    if (m_deviceFd >= 0) {
        close(m_deviceFd);
        m_deviceFd = -1;
    }
}

bool DriverInterface::readPacket(StorageBlock& block) {
    if (m_isKernelDriverLoaded && m_deviceFd >= 0) {
        driver_sensor_packet_t kpacket;
        ssize_t bytesRead = read(m_deviceFd, &kpacket, sizeof(driver_sensor_packet_t));
        if (bytesRead == sizeof(driver_sensor_packet_t)) {
            block.blockId = kpacket.sensor_id;
            block.timestamp = kpacket.timestamp;
            block.metricValue = static_cast<double>(kpacket.raw_value) / 100.0;
            block.severity = static_cast<SeverityLevel>(kpacket.status_flags);
            block.tier = DataTier::HOT_RAM;
            return true;
        }
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mockMutex);
    if (m_mockBuffer.empty()) return false;

    block = m_mockBuffer.front();
    m_mockBuffer.pop();
    m_mockReads++;
    return true;
}

bool DriverInterface::writePacket(const StorageBlock& block) {
    if (m_isKernelDriverLoaded && m_deviceFd >= 0) {
        driver_sensor_packet_t kpacket;
        kpacket.sensor_id = block.blockId;
        kpacket.timestamp = static_cast<uint32_t>(block.timestamp / 1000);
        kpacket.raw_value = static_cast<int32_t>(block.metricValue * 100);
        kpacket.status_flags = static_cast<uint32_t>(block.severity);

        ssize_t bytesWritten = write(m_deviceFd, &kpacket, sizeof(driver_sensor_packet_t));
        return (bytesWritten == sizeof(driver_sensor_packet_t));
    }

    std::lock_guard<std::mutex> lock(m_mockMutex);
    if (m_mockBuffer.size() >= 64) m_mockBuffer.pop();
    m_mockBuffer.push(block);
    m_mockWrites++;
    return true;
}

bool DriverInterface::setSamplingRate(uint32_t samplingRateMs) {
    if (m_isKernelDriverLoaded && m_deviceFd >= 0) {
        int res = ioctl(m_deviceFd, TELEMETRY_IOCTL_SET_SAMPLING_RATE, &samplingRateMs);
        return (res == 0);
    }

    std::lock_guard<std::mutex> lock(m_mockMutex);
    m_mockSamplingRateMs = samplingRateMs;
    std::cout << "[DriverInterface][IOCTL-Software] Sampling rate set to " << samplingRateMs << " ms" << std::endl;
    return true;
}

bool DriverInterface::triggerFlush() {
    if (m_isKernelDriverLoaded && m_deviceFd >= 0) {
        int res = ioctl(m_deviceFd, TELEMETRY_IOCTL_TRIGGER_FLUSH);
        return (res == 0);
    }

    std::lock_guard<std::mutex> lock(m_mockMutex);
    while (!m_mockBuffer.empty()) m_mockBuffer.pop();
    std::cout << "[DriverInterface][IOCTL-Software] Ring buffer flushed." << std::endl;
    return true;
}

bool DriverInterface::getDriverStats(driver_stats_t& stats) {
    if (m_isKernelDriverLoaded && m_deviceFd >= 0) {
        int res = ioctl(m_deviceFd, TELEMETRY_IOCTL_GET_STATS, &stats);
        return (res == 0);
    }

    std::lock_guard<std::mutex> lock(m_mockMutex);
    stats.total_reads = m_mockReads;
    stats.total_writes = m_mockWrites;
    stats.total_ioctls = 0;
    stats.current_sampling_rate_ms = m_mockSamplingRateMs;
    stats.ring_buffer_usage = static_cast<uint32_t>(m_mockBuffer.size());
    return true;
}

} // namespace Telemetry
