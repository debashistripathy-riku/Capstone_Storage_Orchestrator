# Master Makefile for Multi-Tier Storage Orchestrator & Virtual Storage Engine
# Student: Debashis Tripathy (Reg No: 2341020048)
# Course: Wipro COE Capstone Project

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude
LDFLAGS ?= -pthread

BIN_DIR := bin
SRC_DIR := src
TEST_DIR := tests
DRIVER_DIR := driver

COMMON_SRCS := src/DriverInterface.cpp src/SensorManager.cpp src/DataStorage.cpp src/ProcessMonitor.cpp src/TelemetryEngine.cpp src/IPCServer.cpp
MAIN_SRC := src/main.cpp
CLI_SRC := src/cli_main.cpp
UNIT_TEST_SRC := tests/unit_tests.cpp
BENCHMARK_SRC := tests/benchmark.cpp

TARGET_DAEMON := $(BIN_DIR)/storage_orchestrator
TARGET_CLI := $(BIN_DIR)/telemetry_cli
TARGET_TESTS := $(BIN_DIR)/unit_tests
TARGET_BENCHMARK := $(BIN_DIR)/benchmark

.PHONY: all clean driver test benchmark help

all: $(BIN_DIR) $(TARGET_DAEMON) $(TARGET_CLI) $(TARGET_TESTS) $(TARGET_BENCHMARK)
	@echo "============================================================"
	@echo " BUILD COMPLETE! Binaries created in bin/"
	@echo "   Daemon    : ./bin/storage_orchestrator"
	@echo "   CLI Client: ./bin/telemetry_cli"
	@echo "   Unit Tests: ./bin/unit_tests"
	@echo "   Benchmark : ./bin/benchmark"
	@echo "============================================================"

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(TARGET_DAEMON): $(MAIN_SRC) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET_CLI): $(CLI_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET_TESTS): $(UNIT_TEST_SRC) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(TARGET_BENCHMARK): $(BENCHMARK_SRC) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

driver:
	@echo "Building Linux Kernel Driver Module..."
	@if [ -d "/lib/modules/$$(uname -r)/build" ]; then \
		$(MAKE) -C $(DRIVER_DIR); \
	else \
		echo "Kernel headers not installed. Skipping kernel driver .ko build. User-space fallback will be used."; \
	fi

test: $(TARGET_TESTS)
	./$(TARGET_TESTS)

benchmark: $(TARGET_BENCHMARK)
	./$(TARGET_BENCHMARK)

clean:
	rm -rf $(BIN_DIR) *.log test_virtual_storage.log benchmark_virtual_storage.log virtual_storage.log
	@if [ -d "/lib/modules/$$(uname -r)/build" ]; then \
		$(MAKE) -C $(DRIVER_DIR) clean || true; \
	fi
	@echo "Clean completed."

help:
	@echo "Usage:"
	@echo "  make          - Build daemon, CLI client, unit tests, and benchmarks"
	@echo "  make driver   - Build Linux Kernel Driver module (driver/telemetry_driver.ko)"
	@echo "  make test     - Run automated unit test suite"
	@echo "  make benchmark- Run performance latency/throughput benchmark"
	@echo "  make clean    - Remove build outputs and logs"
