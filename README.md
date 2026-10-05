# Multi-Tier Hot/Cold Storage Orchestrator & Virtual Storage Engine

**Author:** Debashis Tripathy  
**Registration No:** 2341020048  
**Evaluation Track:** Wipro Capstone Engineering Track  
**Hardware Cost:** $0.00 *(100% Pure Software Kernel Driver & User-Space Emulator)*  
**Repository:** [Capstone_Storage_Orchestrator](https://github.com/debashistripathy-riku/Capstone_Storage_Orchestrator)

---

## 📌 1. Project Idea & Core Problem (The Midnight Problem)

Modern computer servers process **millions of data requests per second**. Server storage faces a critical bottleneck:
- **RAM Memory:** Ultra-fast (nanoseconds) but expensive and limited in size. Storing all requests in RAM causes **Out-Of-Memory (OOM) kernel crashes**.
- **Hard Disk Drive:** Massive capacity and cheap, but slow (milliseconds). Writing every request directly to disk **slows down system performance to a crawl**.

### 💡 The Solution:
The **Multi-Tier Storage Orchestrator** is an automated software storage engine that bridges this gap. It maintains active data in a fast **Hot RAM Tier** and automatically offloads inactive historical data to a persistent **Cold Disk Tier** (`virtual_storage.log`) when RAM capacity limits are reached.

---

## 🛠️ 2. Tech Stack & Systems Programming Primitives

| Layer / Domain | Technologies & Primitives Used |
| :--- | :--- |
| **Primary Languages** | Modern **C++17** (Daemon Engine & CLI) & **C** (Linux Kernel Device Driver) |
| **Kernel Space** | Character Device Driver (`/dev/hotcold_dev`), Atomic Spinlocks (`spinlock_t`), `ioctl` controls, Sysfs attributes (`/sys/class/hotcold/`) |
| **User Space Core** | Multithreaded daemon engine, Reader-Writer Locks (`std::shared_mutex`), `std::deque` memory queue, RAII smart pointers |
| **Network & IPC** | BSD POSIX TCP Socket Server on **Port 8080**, ANSI Interactive CLI (`telemetry_cli`) |
| **System Inspection** | Pure software parsing of `/proc/stat` (CPU %) and `/proc/meminfo` (RAM MB) |
| **Process Control** | POSIX Signal Handlers (`SIGINT`, `SIGTERM`, `SIGUSR1`) for graceful flushing and shutdown |
| **Build Automation** | GNU `Makefile`, Shell Build Scripts (`build.sh`, `run.sh`), GCC / Clang |

---

## 🏗️ 3. System Architecture & Component Design

```text
                      INCOMING TELEMETRY & DATA REQUESTS
                                      │
                                      ▼
                ┌───────────────────────────────────────────┐
                │   LINUX KERNEL DRIVER (/dev/hotcold_dev)  │
                │   - C Character Driver with Spinlocks     │
                │   - IOCTL Sampling Rate Control           │
                │   - Sysfs Attribute Monitoring Nodes      │
                │   - C++ Fallback Emulator (Non-Root/Mac)  │
                └─────────────────────┬─────────────────────┘
                                      │
                                      ▼
                ┌───────────────────────────────────────────┐
                │      TIER 1: HOT RAM STORAGE QUEUE        │
                │      (std::deque in TelemetryEngine)      │
                │      - Ultra-Fast Nanosecond Access       │
                │      - Max RAM Limit: 100 Data Blocks     │
                └─────────────────────┬─────────────────────┘
                                      │
                         RAM FULL TRIGGER (>= 100 Blocks)
                         Automatic Eviction Engine
                                      │
                                      ▼
                ┌───────────────────────────────────────────┐
                │      TIER 2: COLD DISK FILE LOG           │
                │      (virtual_storage.log File)           │
                │      - Persistent Long-Term Disk Log      │
                │      - Line-by-Line Appended Storage      │
                └───────────────────────────────────────────┘
                                      ▲
                                      │
                 BSD TCP SOCKET SERVER (PORT 8080) & TELEMETRY CLI
                 Commands: STATS, PROC, RATE <ms>, FAULT_ON, FAULT_OFF, PING
```

---

## 💾 4. Dual-Tier Storage Model

1. **Tier 1: Hot RAM Tier (`std::deque`)**
   - Stores live, active telemetry packets in RAM for maximum processing speed.
   - Synchronized using reader-writer locks (`std::shared_mutex`) to allow safe concurrent multithreaded reads and writes.
   - Capacity Threshold: **100 data blocks**.

2. **Tier 2: Cold Disk Tier (`virtual_storage.log`)**
   - Stores evicted historical data blocks permanently on disk.
   - **Eviction Triggers:**
     - **Capacity Eviction:** As soon as RAM queue reaches 100 blocks, the oldest blocks pop automatically and append to `virtual_storage.log`.
     - **Signal Eviction:** Intercepting POSIX signals (`SIGINT`, `SIGTERM`, `SIGUSR1`) flushes all remaining RAM blocks to disk immediately.

---

## 🧮 5. Mathematical Foundations & Benchmark Results

### Mathematical Formulas:
- **Bitwise Modulo Indexing (Kernel Ring Buffer):**
  $$H_{\text{new}} = (H_{\text{old}} + 1) \ \& \ 63$$
- **CPU Percentage Calculation (from `/proc/stat`):**
  $$\text{CPU}_{\%} = \left( \frac{\Delta \text{NonIdle}}{\Delta \text{Total}} \right) \times 100$$
- **RAM Usage Calculation (from `/proc/meminfo`):**
  $$M_{\text{used\_MB}} = \frac{M_{\text{total}} - M_{\text{avail}}}{1024}$$
- **Throughput Calculation:** $P_{\text{ops}} = \frac{10^8}{T_{\text{ms}}}$
- **Latency Calculation:** $L_{\mu\text{s}} = \frac{T_{\text{ms}}}{100}$

### Benchmark & Verification Results:
- **Compilation:** **0 Warnings, 0 Errors**
- **Automated Unit Tests:** **4/4 Passed (100% Success)**
- **System Throughput:** **2,285,200 data blocks / second**
- **Processing Latency:** **0.43 microseconds** per block

---

## 📁 6. Directory Structure

```text
Capstone_Project_Debashis_Tripathy/
├── Makefile                        # Master Makefile
├── build.sh                        # 1-Click build and test script
├── run.sh                          # 1-Click execution script
├── Capstone_Project_Debashis_Tripathy.pptx # 10-Slide PowerPoint Deck
├── Capstone_Project_Story_Debashis_Tripathy.pdf # Project Story PDF
├── Capstone_Project_WH_Guide_Debashis_Tripathy.pdf # WH-Questions PDF Guide
├── driver/                         # C Linux Kernel Device Driver
│   ├── telemetry_driver.c
│   ├── telemetry_driver.h
│   ├── Kbuild
│   └── Makefile
├── include/                        # C++ Header Files
│   ├── Common.hpp
│   ├── DataStorage.hpp
│   ├── DriverInterface.hpp
│   ├── IPCServer.hpp
│   ├── ProcessMonitor.hpp
│   ├── SensorManager.hpp
│   └── TelemetryEngine.hpp
├── src/                            # C++ Source Files
│   ├── DataStorage.cpp
│   ├── DriverInterface.cpp
│   ├── IPCServer.cpp
│   ├── ProcessMonitor.cpp
│   ├── SensorManager.cpp
│   ├── TelemetryEngine.cpp
│   ├── cli_main.cpp
│   └── main.cpp
├── tests/                          # Automated Tests & Benchmark
│   ├── benchmark.cpp
│   └── unit_tests.cpp
└── docs/                           # Documentation & Reports
    ├── CAPSTONE_PROJECT_PRESENTATION.md
    ├── PRESENTATION_SLIDES.html
    ├── PROJECT_EXPLANATION_GUIDE.md
    ├── STAGE_DOCS.md
    └── TERMINAL_OUTPUT_REPORT.md
```

---

## 💻 7. Build, Test, and Execution Instructions

### 1-Click Automated Build & Unit Tests:
```bash
./build.sh
```

### 1-Click Application Launch:
```bash
./run.sh
```

### Manual Build & Test Commands:
```bash
make all        # Compile daemon, CLI, unit tests, and benchmark
make test       # Run automated 4/4 unit test suite
make benchmark  # Execute throughput and latency benchmark
make clean      # Clean object files and output binaries
```

---

## 🖥️ 8. Interactive CLI Commands (`telemetry_cli`)

Connect to the running server daemon on Port 8080 via `./bin/telemetry_cli`:

- `STATS` — Print live Hot RAM queue size and Cold Disk block count.
- `PROC` — Display real-time CPU % and RAM MB metrics parsed from `/proc`.
- `RATE <ms>` — Dynamically change telemetry sampling rate via `ioctl`.
- `FAULT_ON` / `FAULT_OFF` — Toggle stress test fault simulation.
- `PING` — Test server IPC responsiveness.
- `QUIT` — Disconnect CLI session cleanly.

---

## 📜 9. License & Author

**Author:** Debashis Tripathy  
**Registration No:** 2341020048  
**Project Track:** Wipro Capstone Engineering Track  
**GitHub:** [debashistripathy-riku/Capstone_Storage_Orchestrator](https://github.com/debashistripathy-riku/Capstone_Storage_Orchestrator)
