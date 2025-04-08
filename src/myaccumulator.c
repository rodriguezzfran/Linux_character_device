#include <linux/module.h> // Required for all kernel modules, has the macros for licensing, initialization, etc.
#include <linux/kernel.h> // Required for KERN_INFO
#include <linux/init.h> // This have the macros _init and _exit to mark functions as initialization and exit functions
#include <linux/slab.h> // For kmalloc and kfree
#include <linux/fs.h> // For file operations like open, read, write, etc.
#include <linux/uaccess.h> // For copy_to_user and copy_from_user to transfer data between kernel and user space
#include <linux/cdev.h> // For character device operations like cdev_init, cdev_add, etc, used for creating character devices
#include <linux/device.h> // For device creation like device_create, device_destroy, create the node in /dev, etc.