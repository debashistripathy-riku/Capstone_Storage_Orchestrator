

#include "../include/SensorManager.hpp"
#include "../include/DataStorage.hpp"
#include "../include/DriverInterface.hpp"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " PERFORMANCE & LATENCY BENCHMARK SUITE (C++)" << std::endl;
    std::cout << " Student: Debashis Tripathy | Reg No: 2341020048" << std::endl;
    std::cout << "============================================================" << std::endl;

    Telemetry::SensorManager dataGen;
    Telemetry::DataStorage storage(1000, "benchmark_virtual_storage.log");
    Telemetry::DriverInterface driver;
    driver.initialize();

    const int TOTAL_BLOCKS = 100000;
    std::cout << "Benchmarking processing speed for " << TOTAL_BLOCKS << " virtual storage blocks..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TOTAL_BLOCKS; ++i) {
        auto block = dataGen.generateBlock(i % 1000);
        driver.writePacket(block);
        storage.storeBlock(block);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    double avgLatencyUs = (duration.count() * 1000.0) / TOTAL_BLOCKS;
    double throughputOps = (TOTAL_BLOCKS / duration.count()) * 1000.0;

    std::cout << "\nBENCHMARK RESULTS:" << std::endl;
    std::cout << "  Total Blocks Processed  : " << TOTAL_BLOCKS << std::endl;
    std::cout << "  Total Execution Time    : " << duration.count() << " ms" << std::endl;
    std::cout << "  Average Block Latency   : " << avgLatencyUs << " µs (microseconds)" << std::endl;
    std::cout << "  Throughput              : " << throughputOps << " blocks/second" << std::endl;
    std::cout << "============================================================" << std::endl;

    return 0;
}
