

#ifndef TELEMETRY_DRIVER_H
#define TELEMETRY_DRIVER_H

#ifdef __KERNEL__
#include <linux/types.h>
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#include <cstdint>
#include <cstddef>
#endif

#define DEVICE_NAME "telemetry_dev"
#define CLASS_NAME  "telemetry_class"
#define DEVICE_PATH "/dev/telemetry_dev"

// Sensor telemetry structure shared between Kernel Driver and User-space
typedef struct {
    uint32_t sensor_id;    // 1: Temp, 2: RPM, 3: Pressure, 4: Voltage
    uint32_t timestamp;    // Unix timestamp (seconds)
    int32_t  raw_value;    // Sensor reading scaled by 100
    uint32_t status_flags; // 0: Normal, 1: Warning, 2: Critical
} driver_sensor_packet_t;

// Telemetry driver statistics structure
typedef struct {
    uint64_t total_reads;
    uint64_t total_writes;
    uint64_t total_ioctls;
    uint32_t current_sampling_rate_ms;
    uint32_t ring_buffer_usage;
} driver_stats_t;

// IOCTL Command definitions using standard Linux IOCTL magic numbers
#define TELEMETRY_IOCTL_MAGIC 'T'

#define TELEMETRY_IOCTL_SET_SAMPLING_RATE _IOW(TELEMETRY_IOCTL_MAGIC, 1, uint32_t)
#define TELEMETRY_IOCTL_GET_STATS         _IOR(TELEMETRY_IOCTL_MAGIC, 2, driver_stats_t)
#define TELEMETRY_IOCTL_TRIGGER_FLUSH     _IO(TELEMETRY_IOCTL_MAGIC, 3)
#define TELEMETRY_IOCTL_RESET_STATS       _IO(TELEMETRY_IOCTL_MAGIC, 4)

#endif // TELEMETRY_DRIVER_H
