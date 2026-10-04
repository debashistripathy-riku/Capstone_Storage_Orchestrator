/*
 * telemetry_driver.c - Custom Linux Character Device Driver for Hardware Telemetry
 * Author: Debashis Tripathy (Reg No: 2341020048)
 * Course: Wipro COE Capstone Project
 *
 * Concepts Demonstrated:
 * - Linux Kernel Module programming (init_module, cleanup_module)
 * - Character device registration (alloc_chrdev_region, cdev_init)
 * - File operations struct (open, release, read, write, unlocked_ioctl)
 * - Kernel memory allocation (kmalloc, kfree) & Ring Buffer
 * - Kernel-to-User space data copy (copy_to_user, copy_from_user)
 * - Sysfs device attributes (/sys/class/telemetry_class/telemetry_dev/stats)
 * - Spinlock synchronization in kernel space
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>
#include "telemetry_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Debashis Tripathy <2341020048>");
MODULE_DESCRIPTION("Smart Hardware & Sensor Telemetry Character Device Driver");
MODULE_VERSION("1.0");

#define RING_BUFFER_SIZE 64

// Driver State Structure
struct telemetry_dev_state {
    dev_t dev_num;
    struct cdev cdev;
    struct class *dev_class;
    struct device *device;
    spinlock_t lock;
    driver_sensor_packet_t ring_buffer[RING_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    driver_stats_t stats;
};

static struct telemetry_dev_state dev_ctx;

// Sysfs attribute read function
static ssize_t sysfs_stats_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned long flags;
    ssize_t ret;

    spin_lock_irqsave(&dev_ctx.lock, flags);
    ret = sprintf(buf, 
        "Device: %s\n"
        "Total Reads: %llu\n"
        "Total Writes: %llu\n"
        "Total IOCTLs: %llu\n"
        "Sampling Rate (ms): %u\n"
        "Buffer Count: %u/%d\n",
        DEVICE_NAME,
        dev_ctx.stats.total_reads,
        dev_ctx.stats.total_writes,
        dev_ctx.stats.total_ioctls,
        dev_ctx.stats.current_sampling_rate_ms,
        dev_ctx.count, RING_BUFFER_SIZE);
    spin_unlock_irqrestore(&dev_ctx.lock, flags);

    return ret;
}

static DEVICE_ATTR(stats, 0444, sysfs_stats_show, NULL);

// Driver open operation
static int driver_open(struct inode *inode, struct file *file)
{
    pr_info("telemetry_dev: Device opened by process PID %d\n", current->pid);
    return 0;
}

// Driver release operation
static int driver_release(struct inode *inode, struct file *file)
{
    pr_info("telemetry_dev: Device closed\n");
    return 0;
}

// Driver read operation - Reads packet from kernel ring buffer to user space
static ssize_t driver_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    unsigned long flags;
    driver_sensor_packet_t packet;
    size_t packet_size = sizeof(driver_sensor_packet_t);

    if (count < packet_size)
        return -EINVAL;

    spin_lock_irqsave(&dev_ctx.lock, flags);
    if (dev_ctx.count == 0) {
        spin_unlock_irqrestore(&dev_ctx.lock, flags);
        return 0; // Buffer empty
    }

    packet = dev_ctx.ring_buffer[dev_ctx.tail];
    dev_ctx.tail = (dev_ctx.tail + 1) % RING_BUFFER_SIZE;
    dev_ctx.count--;
    dev_ctx.stats.total_reads++;
    spin_unlock_irqrestore(&dev_ctx.lock, flags);

    if (copy_to_user(user_buf, &packet, packet_size))
        return -EFAULT;

    return packet_size;
}

// Driver write operation - Writes user sensor packet into kernel ring buffer
static ssize_t driver_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    unsigned long flags;
    driver_sensor_packet_t packet;
    size_t packet_size = sizeof(driver_sensor_packet_t);

    if (count < packet_size)
        return -EINVAL;

    if (copy_from_user(&packet, user_buf, packet_size))
        return -EFAULT;

    spin_lock_irqsave(&dev_ctx.lock, flags);
    dev_ctx.ring_buffer[dev_ctx.head] = packet;
    dev_ctx.head = (dev_ctx.head + 1) % RING_BUFFER_SIZE;
    if (dev_ctx.count < RING_BUFFER_SIZE) {
        dev_ctx.count++;
    } else {
        // Overwrite oldest if full
        dev_ctx.tail = (dev_ctx.tail + 1) % RING_BUFFER_SIZE;
    }
    dev_ctx.stats.total_writes++;
    spin_unlock_irqrestore(&dev_ctx.lock, flags);

    return packet_size;
}

// Driver ioctl operation - Control commands from user space
static long driver_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned long flags;
    driver_stats_t current_stats;
    uint32_t new_rate;

    spin_lock_irqsave(&dev_ctx.lock, flags);
    dev_ctx.stats.total_ioctls++;
    spin_unlock_irqrestore(&dev_ctx.lock, flags);

    switch (cmd) {
    case TELEMETRY_IOCTL_SET_SAMPLING_RATE:
        if (copy_from_user(&new_rate, (uint32_t __user *)arg, sizeof(uint32_t)))
            return -EFAULT;
        spin_lock_irqsave(&dev_ctx.lock, flags);
        dev_ctx.stats.current_sampling_rate_ms = new_rate;
        spin_unlock_irqrestore(&dev_ctx.lock, flags);
        pr_info("telemetry_dev: Sampling rate updated to %u ms\n", new_rate);
        break;

    case TELEMETRY_IOCTL_GET_STATS:
        spin_lock_irqsave(&dev_ctx.lock, flags);
        current_stats = dev_ctx.stats;
        current_stats.ring_buffer_usage = dev_ctx.count;
        spin_unlock_irqrestore(&dev_ctx.lock, flags);

        if (copy_to_user((driver_stats_t __user *)arg, &current_stats, sizeof(driver_stats_t)))
            return -EFAULT;
        break;

    case TELEMETRY_IOCTL_TRIGGER_FLUSH:
        spin_lock_irqsave(&dev_ctx.lock, flags);
        dev_ctx.head = 0;
        dev_ctx.tail = 0;
        dev_ctx.count = 0;
        spin_unlock_irqrestore(&dev_ctx.lock, flags);
        pr_info("telemetry_dev: Ring buffer flushed\n");
        break;

    case TELEMETRY_IOCTL_RESET_STATS:
        spin_lock_irqsave(&dev_ctx.lock, flags);
        memset(&dev_ctx.stats, 0, sizeof(driver_stats_t));
        dev_ctx.stats.current_sampling_rate_ms = 1000;
        spin_unlock_irqrestore(&dev_ctx.lock, flags);
        pr_info("telemetry_dev: Driver statistics reset\n");
        break;

    default:
        return -ENOTTY;
    }

    return 0;
}

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = driver_open,
    .release        = driver_release,
    .read           = driver_read,
    .write          = driver_write,
    .unlocked_ioctl = driver_ioctl,
};

// Module Initialization
static int __init telemetry_driver_init(void)
{
    int ret;

    spin_lock_init(&dev_ctx.lock);
    dev_ctx.head = 0;
    dev_ctx.tail = 0;
    dev_ctx.count = 0;
    dev_ctx.stats.current_sampling_rate_ms = 1000;

    // Allocate character device region
    ret = alloc_chrdev_region(&dev_ctx.dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("telemetry_dev: Failed to allocate major number\n");
        return ret;
    }

    // Initialize cdev
    cdev_init(&dev_ctx.cdev, &fops);
    dev_ctx.cdev.owner = THIS_MODULE;

    ret = cdev_add(&dev_ctx.cdev, dev_ctx.dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_ctx.dev_num, 1);
        pr_err("telemetry_dev: Failed to add cdev\n");
        return ret;
    }

    // Create device class
    dev_ctx.dev_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(dev_ctx.dev_class)) {
        cdev_del(&dev_ctx.cdev);
        unregister_chrdev_region(dev_ctx.dev_num, 1);
        pr_err("telemetry_dev: Failed to create class\n");
        return PTR_ERR(dev_ctx.dev_class);
    }

    // Create device node under /dev/telemetry_dev
    dev_ctx.device = device_create(dev_ctx.dev_class, NULL, dev_ctx.dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_ctx.device)) {
        class_destroy(dev_ctx.dev_class);
        cdev_del(&dev_ctx.cdev);
        unregister_chrdev_region(dev_ctx.dev_num, 1);
        pr_err("telemetry_dev: Failed to create device node\n");
        return PTR_ERR(dev_ctx.device);
    }

    // Create sysfs attribute node /sys/class/telemetry_class/telemetry_dev/stats
    ret = device_create_file(dev_ctx.device, &dev_attr_stats);
    if (ret) {
        pr_warn("telemetry_dev: Could not create sysfs attribute\n");
    }

    pr_info("telemetry_dev: Loaded successfully. Major: %d, Minor: %d\n", 
            MAJOR(dev_ctx.dev_num), MINOR(dev_ctx.dev_num));
    return 0;
}

// Module Cleanup
static void __exit telemetry_driver_exit(void)
{
    device_remove_file(dev_ctx.device, &dev_attr_stats);
    device_destroy(dev_ctx.dev_class, dev_ctx.dev_num);
    class_destroy(dev_ctx.dev_class);
    cdev_del(&dev_ctx.cdev);
    unregister_chrdev_region(dev_ctx.dev_num, 1);
    pr_info("telemetry_dev: Unloaded successfully\n");
}

module_init(telemetry_driver_init);
module_exit(telemetry_driver_exit);
