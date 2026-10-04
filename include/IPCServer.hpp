

#ifndef IPC_SERVER_HPP
#define IPC_SERVER_HPP

#include "Common.hpp"
#include "TelemetryEngine.hpp"
#include <atomic>
#include <thread>
#include <string>

namespace Telemetry {

class IPCServer {
public:
    IPCServer(TelemetryEngine& engine, uint16_t port = 8080);
    ~IPCServer();

    // Start socket listener loop
    bool start();

    // Stop socket server
    void stop();

private:
    void listenLoop();
    void handleClient(int clientSocket);
    std::string processCommand(const std::string& command);

    TelemetryEngine& m_engine;
    uint16_t m_port;
    int m_serverFd{-1};
    std::atomic<bool> m_running{false};
    std::thread m_listenerThread;
};

} // namespace Telemetry

#endif // IPC_SERVER_HPP
