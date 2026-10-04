# Capstone Project Stages 1-6 Report

**Student Name:** Debashis Tripathy  
**Registration No:** 2341020048  
**Course:** Wipro COE Capstone Project  

---

## Stage 1: Introduction

This project is a Multi-Tier Data Storage Orchestrator written in C++ and C for Linux. It moves data blocks between a fast RAM queue and disk storage to manage memory usage.

Objectives:
- Create a Linux character device driver (`/dev/hotcold_dev`) with ioctl and sysfs.
- Build a multithreaded C++ service to handle storage buffer demotion.
- Monitor CPU and memory usage using `/proc`.
- Provide a CLI client communicating over TCP sockets.

---

## Stage 2: Requirements

### Functional Requirements
- Read and write data blocks to `/dev/hotcold_dev`.
- Implement ioctl commands: `HOTCOLD_IOCTL_SET_WATERMARK`, `HOTCOLD_IOCTL_FLUSH_CACHE`, `HOTCOLD_IOCTL_GET_STATS`.
- Expose driver status at `/sys/class/hotcold_class/hotcold_dev/stats`.
- Store active blocks in RAM and write older blocks to disk when RAM is full.
- Read system metrics from `/proc/stat` and `/proc/meminfo`.
- Handle `SIGINT`, `SIGTERM`, and `SIGUSR1` signals.

### Non-Functional Requirements
- High throughput (> 1,000,000 blocks/sec).
- Low latency (< 1 microsecond).
- Safe memory handling using C++ smart pointers and RAII.

---

## Stage 3: System Design

### Block Diagram

```text
[ Data Generator ] ---> [ Linux Character Driver ] ---> [ C++ Engine ] ---> [ Socket Server ] ---> [ CLI ]
```

### Component Summary
1. `hotcold_driver.c`: Kernel character device driver managing ring buffer and ioctl calls.
2. `DriverInterface.cpp`: Connects user-space C++ code to driver file descriptor, with fallback support.
3. `DataStorage.cpp`: Manages RAM queue and disk file writing.
4. `ProcessMonitor.cpp`: Parses `/proc/stat` and `/proc/meminfo`.
5. `TelemetryEngine.cpp`: Coordinates threads and signal handlers.
6. `IPCServer.cpp`: TCP socket server on port 8080.

---

## Stage 4: Implementation Details

- Kernel module written in C using `alloc_chrdev_region`, `cdev_add`, and spinlocks.
- Engine written in C++17 using `std::thread`, `std::shared_mutex`, and `std::atomic`.
- User-space socket client written in C++ using POSIX socket API.

---

## Stage 5: Testing and Results

- Unit tests check data generation, RAM buffer capacity, disk flushing, ioctl calls, and `/proc` parsing (`./bin/unit_tests`).
- Benchmark test processes 100,000 blocks to measure latency and throughput (`./bin/benchmark`).

---

## Stage 6: Deliverables

- Source code files in `src/`, `include/`, `driver/`, and `tests/`.
- Build scripts `build.sh` and `run.sh`.
- Makefile and documentation.
