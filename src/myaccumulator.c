#include <linux/module.h> // Required for all kernel modules, has the macros for licensing, initialization, etc.
#include <linux/kernel.h> // Required for KERN_INFO
#include <linux/init.h> // This have the macros _init and _exit to mark functions as initialization and exit functions
#include <linux/slab.h> // For kmalloc and kfree
#include <linux/fs.h> // For file operations like open, read, write, etc.
#include <linux/uaccess.h> // For copy_to_user and copy_from_user to transfer data between kernel and user space
#include <linux/cdev.h> // For character device operations like cdev_init, cdev_add, etc, used for creating character devices
#include <linux/device.h> // For device creation like device_create, device_destroy, create the node in /dev, etc.
#include <linux/sched.h> // For scheduling functions
#include <linux/cred.h> // For current macro to get the current task structure

#define DEVICE_NAME "myaccumulator" // Name of the device
#define CLASS_NAME "myaccumulator_class" // Name of the class
#define BUFFER_SIZE 1024 // Size of the buffer for storing data

static dev_t dev_num; // Device number, combination of major and minor numbers to identify a unique device
static struct cdev accumDevice; // Character device structure, defines the device operations and holds the device number
static struct class *accumClass = NULL; // Device class structure, used to create a class for the device
static struct device *accumDeviceObj = NULL; // Device object structure, to create a node in /dev

static long long accumulator = 0; // Accumulator variable to store the sum



 