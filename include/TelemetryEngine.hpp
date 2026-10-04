

#ifndef TELEMETRY_ENGINE_HPP
#define TELEMETRY_ENGINE_HPP

#include "Common.hpp"
#include "DriverInterface.hpp"
#include "SensorManager.hpp"
#include "DataStorage.hpp"
#include "ProcessMonitor.hpp"
#include <atomic>
#include <thread>
#include <mutex>

namespace Telemetry {

class TelemetryEngine {
public:
    TelemetryEngine();
    ~TelemetryEngine();

    bool start();
    void stop();

    bool isRunning() const { return m_running.load(); }
    GatewayStats getStats() const;

    void triggerFaultInjection(bool enable);
    bool updateSamplingRate(uint32_t rateMs);
    std::string getProcessSummary();

    static void setupSignalHandlers(TelemetryEngine* instance);

private:
    void workerThreadLoop();
    void telemetryCollectorLoop();
    void processBlock(const StorageBlock& block);

    DriverInterface m_driverInterface;
    SensorManager m_sensorManager;
    DataStorage m_storage;
    ProcessMonitor m_processMonitor;

    std::atomic<bool> m_running{false};
    std::thread m_workerThread;
    std::thread m_telemetryThread;

    mutable std::mutex m_statsMutex;
    GatewayStats m_stats;

    static TelemetryEngine* s_instance;
    static void handleSignal(int signal);
};

} // namespace Telemetry

#endif // TELEMETRY_ENGINE_HPP
