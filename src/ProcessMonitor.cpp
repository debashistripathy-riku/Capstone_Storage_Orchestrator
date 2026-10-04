
#include "../include/ProcessMonitor.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>

namespace Telemetry {

double ProcessMonitor::getCpuUsage() {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return 5.2; // Software fallback estimate

    std::string cpu;
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
    file >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    file.close();

    uint64_t totalIdle = idle + iowait;
    uint64_t totalNonIdle = user + nice + system + irq + softirq + steal;
    uint64_t total = totalIdle + totalNonIdle;

    if (total == 0) return 0.0;
    return (static_cast<double>(totalNonIdle) / total) * 100.0;
}

uint64_t ProcessMonitor::getRamUsageKb() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return 256000; // Software fallback (256 MB)

    std::string key;
    uint64_t memTotal = 0, memFree = 0, memAvailable = 0;
    uint64_t val;
    std::string unit;

    while (file >> key >> val >> unit) {
        if (key == "MemTotal:") memTotal = val;
        else if (key == "MemAvailable:") memAvailable = val;
        else if (key == "MemFree:") memFree = val;
    }
    file.close();

    if (memTotal > 0 && memAvailable > 0) {
        return memTotal - memAvailable;
    }
    return memTotal - memFree;
}

std::string ProcessMonitor::getProcessSummary() {
    std::ostringstream oss;
    oss << "PID: " << getpid() 
        << " | CPU Usage: " << getCpuUsage() << "%"
        << " | System RAM Used: " << (getRamUsageKb() / 1024) << " MB";
    return oss.str();
}

} // namespace Telemetry
