
#include "../include/TelemetryEngine.hpp"
#include <iostream>
#include <csignal>
#include <chrono>

namespace Telemetry {

TelemetryEngine* TelemetryEngine::s_instance = nullptr;

TelemetryEngine::TelemetryEngine() {
    s_instance = this;
}

TelemetryEngine::~TelemetryEngine() {
    stop();
    s_instance = nullptr;
}

bool TelemetryEngine::start() {
    if (m_running.load()) return true;

    if (!m_driverInterface.initialize()) {
        std::cerr << "[TelemetryEngine] Driver interface initialization failed!" << std::endl;
        return false;
    }

    m_running.store(true);
    setupSignalHandlers(this);

    m_telemetryThread = std::thread(&TelemetryEngine::telemetryCollectorLoop, this);
    m_workerThread = std::thread(&TelemetryEngine::workerThreadLoop, this);

    std::cout << "[TelemetryEngine] Multi-Tier Storage Orchestrator Started." << std::endl;
    return true;
}

void TelemetryEngine::stop() {
    if (!m_running.load()) return;

    std::cout << "[TelemetryEngine] Initiating graceful shutdown..." << std::endl;
    m_running.store(false);

    if (m_telemetryThread.joinable()) m_telemetryThread.join();
    if (m_workerThread.joinable()) m_workerThread.join();

    m_storage.flushToDisk();
    m_driverInterface.closeDevice();
    std::cout << "[TelemetryEngine] Shutdown complete." << std::endl;
}

void TelemetryEngine::telemetryCollectorLoop() {
    while (m_running.load()) {
        auto blocks = m_sensorManager.generateBlockBatch();
        for (const auto& blk : blocks) {
            m_driverInterface.writePacket(blk);
        }

        uint32_t sleepMs = 1000;
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            sleepMs = m_stats.currentSamplingRateMs;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }
}

void TelemetryEngine::workerThreadLoop() {
    while (m_running.load()) {
        StorageBlock blk;
        if (m_driverInterface.readPacket(blk)) {
            processBlock(blk);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void TelemetryEngine::processBlock(const StorageBlock& block) {
    m_storage.storeBlock(block);

    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_stats.totalBlocksStored++;
    m_stats.totalBlocksFlushed++;
    m_stats.driverKernelActive = m_driverInterface.isKernelDriverLoaded();
    m_stats.bytesWrittenToDisk = m_storage.getDiskBytesWritten();
    m_stats.cpuUsagePercent = m_processMonitor.getCpuUsage();
    m_stats.ramUsageKb = m_processMonitor.getRamUsageKb();

    if (block.severity == SeverityLevel::WARNING) {
        std::cout << "[ALERT][WARNING] High metric block ID " << block.blockId 
                  << " value: " << block.metricValue << std::endl;
    } else if (block.severity == SeverityLevel::CRITICAL) {
        std::cout << "[ALERT][CRITICAL] Overflow metric block ID " << block.blockId 
                  << " value: " << block.metricValue << std::endl;
    }
}

GatewayStats TelemetryEngine::getStats() const {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    return m_stats;
}

std::string TelemetryEngine::getProcessSummary() {
    return m_processMonitor.getProcessSummary();
}

void TelemetryEngine::triggerFaultInjection(bool enable) {
    m_sensorManager.setFaultInjection(enable);
    std::cout << "[TelemetryEngine] Software stress test: " 
              << (enable ? "ACTIVATED" : "DEACTIVATED") << std::endl;
}

bool TelemetryEngine::updateSamplingRate(uint32_t rateMs) {
    std::lock_guard<std::mutex> lock(m_statsMutex);
    m_stats.currentSamplingRateMs = rateMs;
    return m_driverInterface.setSamplingRate(rateMs);
}

void TelemetryEngine::setupSignalHandlers(TelemetryEngine* instance) {
    s_instance = instance;
    std::signal(SIGINT, TelemetryEngine::handleSignal);
    std::signal(SIGTERM, TelemetryEngine::handleSignal);
    std::signal(SIGUSR1, TelemetryEngine::handleSignal);
}

void TelemetryEngine::handleSignal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[TelemetryEngine] Signal " << signal << " received. Stopping daemon..." << std::endl;
        if (s_instance) s_instance->stop();
    } else if (signal == SIGUSR1) {
        std::cout << "\n[TelemetryEngine] SIGUSR1 received: Flushing RAM cache to disk." << std::endl;
        if (s_instance) s_instance->m_storage.flushToDisk();
    }
}

} // namespace Telemetry
