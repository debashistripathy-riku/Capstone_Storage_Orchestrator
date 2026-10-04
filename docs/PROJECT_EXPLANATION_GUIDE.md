# Complete Technical Explanation & Viva Guide
## Multi-Tier Hot/Cold Storage Orchestrator & Virtual Storage Engine

**Student Name:** Debashis Tripathy  
**Registration Number:** 2341020048  
**Course:** Wipro COE Capstone Project  
**Target OS:** Linux  

---

## 1. System Architecture Overview

The system is split into two primary layers:
1. **Kernel / Device Layer:** A Linux Character Device Driver (`/dev/hotcold_dev`) written in C. It allocates a kernel ring buffer and handles ioctl commands and sysfs status reporting.
2. **User Space Engine & Service Layer:** A multithreaded C++ application that reads data from the driver, manages a RAM queue (Hot Tier), demotes aged logs to a disk file (Cold Tier), monitors system process metrics via `/proc`, and runs a TCP socket server on Port 8080 for CLI interaction.

```text
[ Data Generator ] ---> [ /dev/hotcold_dev (Kernel/Emulator) ] ---> [ DriverInterface ]
                                                                             |
                                                                             v
                                                                   [ TelemetryEngine ]
                                                                    /         |         \
                                                      [ RAM Buffer ]   [ Disk Log ]   [ /proc Parser ]
                                                           |                |
                                                      (Hot Tier)       (Cold Tier)
                                                           \                /
                                                            +-------+------+
                                                                    |
                                                                    v
                                                            [ IPCServer (8080) ] <---> [ telemetry_cli ]
```

---

## 2. Component-by-Component Code Explanation

### 2.1 Linux Kernel Character Driver (`driver/telemetry_driver.c`)
- **Character Device Registration:** `alloc_chrdev_region()` dynamically allocates a major and minor number. `cdev_init()` binds the `file_operations` structure (`fops`), and `cdev_add()` registers the device with the kernel VFS.
- **Kernel Ring Buffer:** Uses a 64-entry array `ring_buffer` protected by `spin_lock_irqsave()` and `spin_unlock_irqrestore()`. This prevents race conditions between process context reads/writes and interrupt context.
- **Copying Data across Kernel Boundary:**
  - `copy_from_user()` safely copies sensor packets from user space into kernel space memory during `driver_write()`.
  - `copy_to_user()` transfers data packets from kernel space to user space during `driver_read()`.
- **IOCTL Handling (`driver_ioctl`):**
  - `TELEMETRY_IOCTL_SET_SAMPLING_RATE`: Updates sampling interval in milliseconds.
  - `TELEMETRY_IOCTL_GET_STATS`: Returns read, write, and ioctl execution counts.
  - `TELEMETRY_IOCTL_TRIGGER_FLUSH`: Resets buffer pointers to flush active entries.
- **Sysfs Node:** `device_create_file()` registers `/sys/class/hotcold_class/hotcold_dev/stats` allowing users to view driver stats via simple `cat` commands.

### 2.2 Driver Interface & User-Space Fallback (`src/DriverInterface.cpp`)
- `initialize()` attempts to open `/dev/hotcold_dev` using low-level POSIX `open(DEVICE_PATH, O_RDWR)`.
- **Fallback Mechanism:** If `/dev/hotcold_dev` cannot be opened (e.g. running on non-root environment or WSL without custom kernel headers), it seamlessly activates a C++ user-space ring buffer emulator (`std::queue<StorageBlock>`). This guarantees the code runs anywhere without crashing.

### 2.3 Storage Tiering Engine (`src/DataStorage.cpp`)
- **Hot RAM Tier:** Stores active incoming blocks in a `std::deque<StorageBlock>` up to a defined capacity limit (e.g., 100 blocks).
- **Cold Disk Tier:** When RAM capacity is reached, the oldest block is popped from the front of the queue, its tier attribute is updated to `COLD_DISK`, and its contents are written out to a persistent log file (`virtual_storage.log`) using `std::ofstream`.
- **Thread Safety:** Access to the `std::deque` and file handle is synchronized using `std::shared_mutex`. Readers acquire `std::shared_lock` (allowing simultaneous reads), while writers acquire `std::unique_lock` (exclusive access).

### 2.4 Linux System Process Monitor (`src/ProcessMonitor.cpp`)
- Parses `/proc/stat` to calculate CPU utilization:
  $$\text{CPU Usage \%} = \frac{\text{NonIdle Ticks}}{\text{Total Ticks}} \times 100$$
- Parses `/proc/meminfo` to extract `MemTotal` and `MemAvailable`, computing consumed RAM in Kilobytes.
- Operates entirely in software without external hardware dependencies.

### 2.5 Core Daemon & Signal Management (`src/TelemetryEngine.cpp`)
- Spawns two background worker threads via `std::thread`:
  1. `telemetryCollectorLoop`: Generates data blocks and sends them to the driver interface.
  2. `workerThreadLoop`: Reads blocks from the driver interface, evaluates severity, and passes them to `DataStorage`.
- **Signal Handling:**
  - `SIGINT` / `SIGTERM`: Triggers graceful daemon shutdown, joins threads, and flushes RAM data to disk.
  - `SIGUSR1`: Triggers an immediate manual RAM cache flush without stopping the daemon.

### 2.6 POSIX TCP Socket IPC Server (`src/IPCServer.cpp` & `src/cli_main.cpp`)
- Binds to `0.0.0.0:8080` using POSIX socket system calls (`socket`, `bind`, `listen`, `accept`).
- Spawns detached threads to handle individual client socket connections.
- Processes commands (`STATS`, `PROC`, `RATE`, `FAULT_ON`, `FAULT_OFF`, `PING`) and returns text responses.
- `src/cli_main.cpp` provides an interactive prompt (`storage> `) with ANSI terminal colors and timeout protection (`SO_RCVTIMEO`).

---

## 3. How to Answer Viva & Interview Questions

### Q1: Why did you use `std::shared_mutex` instead of `std::mutex`?
> **Answer:** *"In a multi-tier storage engine, reading stats happens frequently, while writing new blocks occurs periodically. `std::shared_mutex` allows multiple threads to read RAM buffers concurrently using `std::shared_lock`, improving throughput. `std::unique_lock` is only used during data insertion or disk flushing."*

### Q2: How does kernel to user space data transfer work in your driver?
> **Answer:** *"Kernel space and user space memory are isolated. In `driver_write()`, I use `copy_from_user()` to move user struct data into the kernel ring buffer. In `driver_read()`, I use `copy_to_user()` to copy data from kernel memory into the user buffer."*

### Q3: How do you handle non-root Linux environments?
> **Answer:** *"The `DriverInterface` class checks if `/dev/hotcold_dev` can be opened. If missing or permission is denied, it switches to a user-space C++ queue fallback emulator, ensuring 100% execution compatibility."*

### Q4: How is data demotion triggered?
> **Answer:** *"When `storeBlock()` is called, it checks if the RAM queue size exceeds `m_maxRamCapacity`. If full, the oldest block at the front is popped, tagged as `COLD_DISK`, and formatted into `virtual_storage.log`."*

---

**Prepared by:** Debashis Tripathy (Reg No: 2341020048)
