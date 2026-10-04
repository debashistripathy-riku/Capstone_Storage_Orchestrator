
#ifndef PROCESS_MONITOR_HPP
#define PROCESS_MONITOR_HPP

#include "Common.hpp"
#include <string>

namespace Telemetry {

class ProcessMonitor {
public:
    ProcessMonitor() = default;
    ~ProcessMonitor() = default;

    // Read CPU usage percentage from /proc/stat
    double getCpuUsage();

    // Read total system RAM usage in Kilobytes from /proc/meminfo
    uint64_t getRamUsageKb();

    // Get active process status string
    std::string getProcessSummary();
};

} // namespace Telemetry

#endif // PROCESS_MONITOR_HPP
