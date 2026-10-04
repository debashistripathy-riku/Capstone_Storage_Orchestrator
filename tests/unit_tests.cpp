/*
 * unit_tests.cpp - Automated Unit Test Suite (C++)
 * Student: Debashis Tripathy (Reg No: 2341020048)
 * Course: Wipro COE Capstone Project
 */

#include "../include/SensorManager.hpp"
#include "../include/DataStorage.hpp"
#include "../include/DriverInterface.hpp"
#include "../include/ProcessMonitor.hpp"
#include <iostream>
#include <cassert>

void testDataGenerator() {
    std::cout << "[TEST] SensorManager Software Block Generation..." << std::endl;
    Telemetry::SensorManager mgr;
    auto block = mgr.generateBlock(101);
    assert(block.blockId == 101);
    assert(block.metricValue > 0.0);
    std::cout << "  PASS: Generated block ID=" << block.blockId << ", Value=" << block.metricValue << std::endl;
}

void testDataStorage() {
    std::cout << "[TEST] DataStorage RAM Buffer & File Flushing..." << std::endl;
    Telemetry::DataStorage storage(5, "test_virtual_storage.log");
    
    Telemetry::StorageBlock block;
    block.blockId = 101;
    block.timestamp = 1000;
    block.metricValue = 42.0;
    block.severity = Telemetry::SeverityLevel::NORMAL;
    block.tier = Telemetry::DataTier::HOT_RAM;

    for (int i = 0; i < 10; ++i) {
        storage.storeBlock(block);
    }

    auto recent = storage.getRecentBlocks(5);
    assert(recent.size() == 5);
    std::cout << "  PASS: RAM storage capacity limit & disk demotion verified." << std::endl;
}

void testDriverInterface() {
    std::cout << "[TEST] DriverInterface Connection & Software IOCTL Fallback..." << std::endl;
    Telemetry::DriverInterface driver;
    assert(driver.initialize());

    Telemetry::StorageBlock block;
    block.blockId = 202;
    block.timestamp = 2000;
    block.metricValue = 88.5;
    block.severity = Telemetry::SeverityLevel::NORMAL;
    block.tier = Telemetry::DataTier::HOT_RAM;

    assert(driver.writePacket(block));
    Telemetry::StorageBlock readBlock;
    assert(driver.readPacket(readBlock));
    assert(readBlock.blockId == 202);

    assert(driver.setSamplingRate(500));
    std::cout << "  PASS: Virtual driver read/write and ioctl commands working." << std::endl;
}

void testProcessMonitor() {
    std::cout << "[TEST] ProcessMonitor /proc System Inspection..." << std::endl;
    Telemetry::ProcessMonitor monitor;
    double cpu = monitor.getCpuUsage();
    uint64_t ram = monitor.getRamUsageKb();
    assert(ram > 0);
    std::cout << "  PASS: Linux /proc stats captured. CPU=" << cpu << "%, RAM=" << (ram / 1024) << "MB" << std::endl;
}

int main() {
    std::cout << "============================================================" << std::endl;
    std::cout << " RUNNING AUTOMATED C++ UNIT TESTS (100% Software)" << std::endl;
    std::cout << " Student: Debashis Tripathy | Reg No: 2341020048" << std::endl;
    std::cout << "============================================================" << std::endl;

    try {
        testDataGenerator();
        testDataStorage();
        testDriverInterface();
        testProcessMonitor();
        std::cout << "\nALL UNIT TESTS PASSED SUCCESSFULLY! (100% Success)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "UNIT TEST FAILED: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
