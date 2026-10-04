/*
 * IPCServer.cpp - POSIX TCP Socket Server Implementation (C++)
 * Student: Debashis Tripathy (Reg No: 2341020048)
 * Course: Wipro COE Capstone Project
 */

#include "../include/IPCServer.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

namespace Telemetry {

IPCServer::IPCServer(TelemetryEngine& engine, uint16_t port)
    : m_engine(engine), m_port(port) {}

IPCServer::~IPCServer() {
    stop();
}

bool IPCServer::start() {
    m_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverFd < 0) {
        std::cerr << "[IPCServer] Failed to create POSIX socket." << std::endl;
        return false;
    }

    int opt = 1;
    setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(m_port);

    if (bind(m_serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[IPCServer] Socket bind failed on port " << m_port << std::endl;
        close(m_serverFd);
        return false;
    }

    if (listen(m_serverFd, 5) < 0) {
        std::cerr << "[IPCServer] Socket listen failed." << std::endl;
        close(m_serverFd);
        return false;
    }

    m_running.store(true);
    m_listenerThread = std::thread(&IPCServer::listenLoop, this);
    std::cout << "[IPCServer] Listening on POSIX TCP port " << m_port << std::endl;
    return true;
}

void IPCServer::stop() {
    if (!m_running.load()) return;
    m_running.store(false);

    if (m_serverFd >= 0) {
        shutdown(m_serverFd, SHUT_RDWR);
        close(m_serverFd);
        m_serverFd = -1;
    }

    if (m_listenerThread.joinable()) {
        m_listenerThread.join();
    }
}

void IPCServer::listenLoop() {
    while (m_running.load()) {
        sockaddr_in clientAddr{};
        socklen_t addrLen = sizeof(clientAddr);
        int clientSocket = accept(m_serverFd, (struct sockaddr*)&clientAddr, &addrLen);

        if (clientSocket < 0) {
            if (!m_running.load()) break;
            continue;
        }

        std::thread(&IPCServer::handleClient, this, clientSocket).detach();
    }
}

void IPCServer::handleClient(int clientSocket) {
    char buffer[1024] = {0};
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        std::string command(buffer, bytesRead);
        std::string response = processCommand(command);
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    close(clientSocket);
}

std::string IPCServer::processCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "STATS") {
        auto stats = m_engine.getStats();
        std::ostringstream oss;
        oss << "=== MULTI-TIER STORAGE ORCHESTRATOR STATS ===\n"
            << "Virtual Kernel Driver: " << (stats.driverKernelActive ? "LOADED (/dev/hotcold_dev)" : "SOFTWARE EMULATOR ($0 Cost)") << "\n"
            << "Total Blocks Stored  : " << stats.totalBlocksStored << "\n"
            << "Total Blocks Flushed : " << stats.totalBlocksFlushed << "\n"
            << "Disk Bytes Written   : " << stats.bytesWrittenToDisk << " bytes\n"
            << "Current Sampling Rate: " << stats.currentSamplingRateMs << " ms\n"
            << "System CPU Usage     : " << stats.cpuUsagePercent << " %\n"
            << "System RAM Used      : " << (stats.ramUsageKb / 1024) << " MB\n";
        return oss.str();
    } else if (cmd == "PROC") {
        return "SOFTWARE PROCESS SUMMARY: " + m_engine.getProcessSummary() + "\n";
    } else if (cmd == "RATE") {
        uint32_t newRateMs = 1000;
        if (iss >> newRateMs) {
            m_engine.updateSamplingRate(newRateMs);
            return "SUCCESS: Sampling rate set to " + std::to_string(newRateMs) + " ms\n";
        }
        return "ERROR: Invalid rate value\n";
    } else if (cmd == "FAULT_ON") {
        m_engine.triggerFaultInjection(true);
        return "SUCCESS: Software stress test ACTIVATED\n";
    } else if (cmd == "FAULT_OFF") {
        m_engine.triggerFaultInjection(false);
        return "SUCCESS: Software stress test DEACTIVATED\n";
    } else if (cmd == "PING") {
        return "PONG: Multi-Tier Storage Daemon Online\n";
    }

    return "ERROR: Unknown command. Available: STATS, PROC, RATE <ms>, FAULT_ON, FAULT_OFF, PING\n";
}

} // namespace Telemetry
