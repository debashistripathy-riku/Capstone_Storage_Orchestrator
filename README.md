# Multi-Tier Storage Orchestrator

**Student:** Debashis Tripathy  
**Registration No:** 2341020048  
**Course:** Wipro COE Capstone Project  

---

## About the Project

This project implements a multi-tier data storage orchestrator on Linux using C++ and C. It manages data movement between RAM and disk storage.

The implementation covers topics from the 20-day training:
- Linux Device Driver (Character device `/dev/hotcold_dev` with ioctl and sysfs)
- POSIX System Programming (`std::thread`, `std::shared_mutex`, signals, file I/O)
- Network Programming (TCP socket server on port 8080)
- Process monitoring via `/proc/stat` and `/proc/meminfo`
- C++ OOP with smart pointers, RAII, and templates

---

## Directory Structure

```text
.
├── Makefile
├── build.sh
├── run.sh
├── driver/
│   ├── hotcold_driver.c
│   ├── hotcold_driver.h
│   ├── Kbuild
│   └── Makefile
├── include/
│   ├── Common.hpp
│   ├── DataStorage.hpp
│   ├── DriverInterface.hpp
│   ├── IPCServer.hpp
│   ├── ProcessMonitor.hpp
│   ├── SensorManager.hpp
│   └── TelemetryEngine.hpp
├── src/
│   ├── DataStorage.cpp
│   ├── DriverInterface.cpp
│   ├── IPCServer.cpp
│   ├── ProcessMonitor.cpp
│   ├── SensorManager.cpp
│   ├── TelemetryEngine.cpp
│   ├── cli_main.cpp
│   └── main.cpp
├── tests/
│   ├── benchmark.cpp
│   └── unit_tests.cpp
└── docs/
    ├── STAGE_DOCS.md
    └── TERMINAL_OUTPUT_REPORT.md
```

---

## How to Build and Run

### 1. Build and Test
Run the script to compile and execute tests:
```bash
./build.sh
```

### 2. Run Application
Run the daemon and CLI interface:
```bash
./run.sh
```

### 3. Manual Build Commands
```bash
make all        # Compile everything
make test       # Run unit tests
make benchmark  # Run performance benchmark
make clean      # Clean output binaries
```

---

## CLI Commands

When connected via `./bin/telemetry_cli`:
- `STATS`       - Print storage and driver stats
- `PROC`        - Print CPU and memory usage from `/proc`
- `RATE <ms>`   - Change sampling rate via ioctl
- `FAULT_ON`   - Enable stress test mode
- `FAULT_OFF`  - Disable stress test mode
- `PING`        - Test server connection
- `QUIT`        - Exit CLI

---

## Project Stages Summary

1. **Stage 1:** Problem definition and project scope.
2. **Stage 2:** Functional requirements (driver ioctl, sysfs node, storage demotion, socket server).
3. **Stage 3:** Architecture design and UML diagrams (see `docs/STAGE_DOCS.md`).
4. **Stage 4:** C kernel driver and C++ user-space implementation.
5. **Stage 5:** Unit testing and benchmark evaluation.
6. **Stage 6:** Final code packaging and documentation.
