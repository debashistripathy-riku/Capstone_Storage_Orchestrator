/*
 * main.cpp - Entrypoint for Storage Orchestrator daemon
 * Debashis Tripathy (2341020048)
 */

#include "../include/TelemetryEngine.hpp"
#include "../include/IPCServer.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    std::cout << "Starting Multi-Tier Storage Orchestrator" << std::endl;
    std::cout << "Student: Debashis Tripathy (2341020048)" << std::endl;

    Telemetry::TelemetryEngine engine;
    if (!engine.start()) {
        std::cerr << "Error starting engine." << std::endl;
        return 1;
    }

    Telemetry::IPCServer ipcServer(engine, 8080);
    if (!ipcServer.start()) {
        std::cerr << "Error starting socket server." << std::endl;
        engine.stop();
        return 1;
    }

    std::cout << "Daemon running. Press Ctrl+C to stop." << std::endl;

    while (engine.isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ipcServer.stop();
    engine.stop();

    std::cout << "Daemon stopped." << std::endl;
    return 0;
}
