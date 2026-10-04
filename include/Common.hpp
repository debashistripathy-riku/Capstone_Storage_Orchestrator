/*
 * Common.hpp - Data Structures, Enums, and System Metrics Definitions (C++)
 * Student: Debashis Tripathy (Reg No: 2341020048)
 * Course: Wipro COE Capstone Project
 * Project: Multi-Tier Hot/Cold Data Storage Orchestrator & Virtual Storage Engine
 */

#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <chrono>
#include <iomanip>

namespace Telemetry {

// Storage Block Category / Data Tier
enum class DataTier : uint32_t {
    HOT_RAM  = 1,
    COLD_DISK = 2
};

// Data Severity / Alert Level
enum class SeverityLevel : uint32_t {
    NORMAL   = 0,
    WARNING  = 1,
    CRITICAL = 2
};

// Virtual Storage Data Block Packet (Pure Software Structure)
struct StorageBlock {
    uint32_t blockId;        // Block Identifier
    uint64_t timestamp;      // Millisecond Timestamp
    double   metricValue;    // Simulated metric / data value
    SeverityLevel severity;  // Status flag
    DataTier tier;           // HOT_RAM or COLD_DISK

    std::string getTierString() const {
        return (tier == DataTier::HOT_RAM) ? "HOT_RAM" : "COLD_DISK";
    }

    std::string getSeverityString() const {
        switch (severity) {
            case SeverityLevel::NORMAL:   return "NORMAL";
            case SeverityLevel::WARNING:  return "WARNING";
            case SeverityLevel::CRITICAL: return "CRITICAL";
            default:                      return "UNKNOWN";
        }
    }
};

// System & Virtual Storage Engine Statistics
struct GatewayStats {
    uint64_t totalBlocksStored{0};
    uint64_t totalBlocksFlushed{0};
    uint64_t ramHits{0};
    uint64_t diskWrites{0};
    uint64_t bytesWrittenToDisk{0};
    uint32_t currentSamplingRateMs{1000};
    bool driverKernelActive{false};
    
    // Software System Process Metrics (from /proc)
    double cpuUsagePercent{0.0};
    uint64_t ramUsageKb{0};
};

} // namespace Telemetry

#endif // COMMON_HPP
