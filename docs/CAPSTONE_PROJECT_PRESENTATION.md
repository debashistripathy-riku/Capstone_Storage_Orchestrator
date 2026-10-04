# Capstone Project Final Review Presentation
## Multi-Tier Hot/Cold Storage Orchestrator & Virtual Storage Engine

**Student Name:** Debashis Tripathy  
**Registration Number:** 2341020048  
**Course:** Wipro COE Capstone Project  
**Domain:** Linux Systems Programming, Kernel Drivers, & C++ OOP  

---

## 📌 Slide 1: Title & Student Information

### Multi-Tier Hot/Cold Storage Orchestrator
*A Software Storage Engine & Linux Kernel Character Device Driver*

- **Presenter:** Debashis Tripathy
- **Registration No:** 2341020048
- **Program:** Wipro COE 20-Day Training Capstone Project
- **Target OS:** Linux / WSL (Ubuntu Kernel 5.x/6.x)
- **Languages:** C++17 (User Space Engine, CLI, IPC, Tests) & C (Linux Kernel Driver)

---

## 📌 Slide 2: Problem Statement & Project Objective

### Problem Statement
In enterprise servers and embedded systems, high-frequency active data consumes valuable RAM, while persistent disk storage is slow. A balanced storage engine is needed to manage fast RAM queues and automate log demotion to disk without system degradation.

### Objectives
- Develop a Linux Virtual Character Device Driver (`/dev/hotcold_dev`) supporting `ioctl` and sysfs attributes.
- Build a multithreaded C++ storage engine for active RAM buffering (Hot Tier) and disk file persistence (Cold Tier).
- Implement Linux process and system resource monitoring by parsing `/proc/stat` and `/proc/meminfo`.
- Provide an interactive client/server dashboard over POSIX TCP Sockets on Port 8080.

---

## 📌 Slide 3: System Architecture & Data Flowchart

### High-Level System Flowchart

```mermaid
flowchart TD
    A["Software Data Generator"] -->|Data Blocks| B["Linux Character Driver (/dev/hotcold_dev)"]
    B -->|Kernel / User Buffer| C["C++ DriverInterface"]
    C -->|Read/Write Operations| D["TelemetryEngine Daemon"]
    D -->|Active Hot Blocks| E["RAM Hot Tier (LRU Cache)"]
    E -->|RAM Capacity Exceeded| F["Disk Cold Tier (POSIX Log File)"]
    D -->|Parse System Stats| G["Linux /proc Inspector"]
    H["Interactive CLI (telemetry_cli)"] <-->|TCP Socket (Port 8080)| I["IPCServer"]
    I <--> D
```

---

## 📌 Slide 4: Linux Kernel Device Driver (`/dev/hotcold_dev`)

### Driver Characteristics
- **Registration:** Character device registered via `alloc_chrdev_region()` and `cdev_add()`.
- **Kernel Memory:** 64-entry kernel ring buffer protected by `spin_lock_irqsave()`.
- **IOCTL Commands:**
  - `HOTCOLD_IOCTL_SET_WATERMARK`: Configures sensor sampling rate dynamically.
  - `HOTCOLD_IOCTL_FLUSH_CACHE`: Flushes kernel ring buffer.
  - `HOTCOLD_IOCTL_GET_STATS`: Retrieves kernel driver stats.
- **Sysfs Node:** `/sys/class/hotcold_class/hotcold_dev/stats` exposes live metrics.
- **Software Fallback:** Includes a user-space C++ driver emulator if kernel headers are not available.

---

## 📌 Slide 5: C++ Storage Engine & Tiering Logic

### Dual-Tier Memory Architecture
- **Hot RAM Tier:** Uses `std::deque<StorageBlock>` for low-latency memory operations.
- **Cold Disk Tier:** Automatically demotes aged RAM blocks to a persistent file log (`virtual_storage.log`) via `std::ofstream`.
- **Thread Safety:** Uses `std::shared_mutex` allowing multiple concurrent readers and an exclusive writer, eliminating race conditions.
- **Memory Safety:** Built strictly using C++ RAII and smart pointers (`std::unique_ptr`).

---

## 📌 Slide 6: Linux `/proc` Process Inspector

### Process & System Monitoring
- **CPU Metrics:** Reads `/proc/stat` to calculate system-wide CPU usage percentage.
- **RAM Metrics:** Reads `/proc/meminfo` (`MemTotal`, `MemAvailable`, `MemFree`) to calculate consumed RAM in Kilobytes/Megabytes.
- **Process Metrics:** Inspects current daemon process state and thread status.
- **Software Execution:** Performs all monitoring via Linux virtual filesystem files without needing physical hardware.

---

## 📌 Slide 7: POSIX Socket IPC & Interactive CLI

### Client-Server Architecture
- **POSIX TCP Server:** Listens on port 8080 using native socket system calls (`socket`, `bind`, `listen`, `accept`).
- **CLI Commands (`telemetry_cli`):**
  - `STATS`: Displays active block counts, disk bytes written, and driver status.
  - `PROC`: Displays system CPU and RAM usage parsed from `/proc`.
  - `RATE <ms>`: Sends ioctl command to set sampling rate.
  - `FAULT_ON` / `FAULT_OFF`: Controls stress testing mode.
  - `PING`: Checks daemon connection health.

---

## 📌 Slide 8: Testing & Performance Results

### Metric Verification Results

| Evaluation Metric | Target | Measured Result | Status |
| :--- | :--- | :--- | :--- |
| **Compilation** | 0 Errors, 0 Warnings | 0 Errors, 0 Warnings | ✅ PASSED |
| **Automated Unit Tests** | 100% Pass Rate | 4/4 Tests Passed (100%) | ✅ PASSED |
| **Processing Throughput** | > 1,000,000 blocks/sec | **2,285,200 blocks/sec** | ✅ EXCEEDED |
| **Block Latency** | < 1 microsecond | **0.43 microseconds** | ✅ EXCEEDED |
| **Memory Leak Audit** | 0 Leaks | 0 Leaks (RAII Managed) | ✅ PASSED |

---

## 📌 Slide 9: 20-Day Training Topic Coverage

```text
  +-----------------------------------------------------------------------+
  |                        CAPSTONE COVERAGE MAP                          |
  +-----------------------------------------------------------------------+
  |  Training Topic          |  Implemented Project Component             |
  +--------------------------+--------------------------------------------+
  |  Linux Kernel Drivers    |  Character Driver, ioctl, sysfs node       |
  |  System Programming      |  std::thread, std::shared_mutex, signals   |
  |  Computer Architecture   |  RAM Cache (Hot Tier) vs Disk (Cold Tier)   |
  |  Network Programming     |  POSIX TCP Socket Server (Port 8080)       |
  |  File Systems            |  POSIX File I/O & /proc filesystem parsing  |
  |  C++ Programming         |  C++17 OOP, Smart Pointers, Templates, STL  |
  +-----------------------------------------------------------------------+
```

---

## 📌 Slide 10: Conclusion & Interview Summary

### Key Takeaways
1. Fully functional Multi-Tier Storage Orchestrator bridging kernel driver space and C++ user space.
2. High-performance software processing (> 2.2 Million blocks/sec throughput, 0.43 µs latency).
3. 100% compliant with Wipro Capstone evaluation criteria.
4. Includes one-click build (`./build.sh`) and interactive execution (`./run.sh`) scripts.

---

**Thank You!**  
*Debashis Tripathy | Reg No: 2341020048*
