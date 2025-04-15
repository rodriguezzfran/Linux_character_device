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
#include <linux/mutex.h> // For mutex operations

#define DEVICE_NAME "myaccumulator" // Name of the device
#define CLASS_NAME "myaccumulator_class" // Name of the class
#define BUFFER_SIZE 1024 // Size of the buffer for storing data

static dev_t dev_num; // Device number, combination of major and minor numbers to identify a unique device
static struct cdev accumDevice; // Character device structure, defines the device operations and holds the device number
static struct class *accumClass = NULL; // Device class structure, used to create a class for the device

static long long accumulator = 0; // Accumulator variable to store the sum

// Mutex to protect the accumulator variable from concurrent access
static DEFINE_MUTEX(accumulator_mutex);

/**
 * @brief Handles attempts to open the character device.
 *
 * Only allows access if the calling process has root privileges.
 * Logs access using pr_info and pr_warn.
 *
 * @param inode Pointer to the inode structure associated with the device.
 * @param file Pointer to the file structure representing the open file.
 * 
 * @return 0 on success, or -EACCES if access is denied (not root).
 */
static int device_open(struct inode *inode, struct file *file) {

    kuid_t uid = current_uid();
    pid_t pid = current->pid;
    const char *cmd = current->comm;

    // If everything is ok, we can open the device
    pr_info("myaccumulator: Device opened - PID %d, UID %d, comando: %s\n",
            pid, uid.val, cmd);
    return 0;

}

/**
 * @brief Handles the release operation on the device.
 *
 * Logs the release event with the PID of the calling process.
 *
 * @param inode Pointer to the inode structure associated with the device.
 * @param file Pointer to the file structure representing the open file.
 * 
 * @return Always returns 0.
 */
static int dev_release(struct inode *inode, struct file *file) {
    
    pr_info("myaccumulator: Device closed by PID %d\n", current->pid);
    
    return 0;
}

/**
 * @brief Read the current value of the accumulator from the device.
 * 
 * This function copies the current value of the accumulator to user space
 * when the device is read. It returns the value only once per open-read cycle
 *
 * @param file Pointer to the file object (unused but required by the function signature).
 * @param buffer User space buffer where the data will be copied.
 * @param len Number of bytes requested to read.
 * @param offset Offset in the file (used to return 0 on second read).
 * 
 * @return ssize_t Number of bytes read on success, 0 if already read (EOF), 
 *         or a negative error code on failure.
 */
static ssize_t dev_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    
    char *user_data; // Buffer to store the data to be read
    int size;

    if (*offset > 0) {
        return 0; // Simulate EOF on subsequent reads
    }

    user_data = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!user_data) {
        pr_err("myaccumulator: Failed to allocate memory for temp buffer\n");
        return -ENOMEM;
    }

    // Lock the mutex to protect the accumulator variable
    mutex_lock(&accumulator_mutex);

    // snprintf is used to format the string, it returns the number of bytes written
    size = snprintf(user_data, BUFFER_SIZE, "%lld\n", accumulator);

    mutex_unlock(&accumulator_mutex); // Unlock the mutex

    // Check if the size is greater than the buffer size
    if (size < 0) {
        kfree(user_data); // Free the kernel buffer
        pr_err("myaccumulator: Error formatting the string\n");
        return -EFAULT;
    }

    // Copy the data to user space
    if (copy_to_user(buffer, user_data, size)) {
        kfree(user_data); // Free the kernel buffer
        pr_err("myaccumulator: Error copying data to user space\n");
        return -EFAULT;
    }

    // Update the offset
    *offset += size;
    kfree(user_data); // Free the kernel buffer

    // put a message in the kernel log
    pr_info("myaccumulator: Read %lld from device\n", accumulator);
    return size; // Return the number of bytes read
}

/**
 * @brief Handles write operations to the device.
 *
 * Receives a string from the user space representing an integer value,
 * converts it to a long, and adds it to the internal accumulator.
 *
 * @param file   Pointer to the file structure.
 * @param buffer Pointer to the user-space buffer containing the data.
 * @param len    Length of the data to write.
 * @param offset Pointer to the file offset (not used but required for the function signature).
 *  
 * @return Number of bytes written on success, or a negative error code on failure:
 *         - -ENOMEM if memory allocation fails.
 *         - -EFAULT if copy_from_user fails.
 *         - -EINVAL if string conversion fails.
 */
static ssize_t dev_write (struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    
    char *kbuf; // Kernel buffer to store the data from user space
    long  value; // Value to accumulate

    kbuf = kmalloc(len + 1, GFP_KERNEL); // Allocate memory for the kernel buffer, the +1 is for the null terminator at the beginning
    if (!kbuf) {
        pr_err("myaccumulator: Error allocating memory for kernel buffer\n");
        return -ENOMEM; // Return error if memory allocation fails
    }

    // Copy the data from user space to kernel space
    if (copy_from_user(kbuf, buffer, len)) {
        pr_err("myaccumulator: Error copying data from user space\n");
        kfree(kbuf); // Free the kernel buffer
        return -EFAULT; // Return error if copy fails
    }

    kbuf[len] = '\0'; // Null terminate the string

    // Convert the string to a long integer
    if (kstrtol(kbuf, 10, &value) == 0) {

        // Lock the mutex to protect the accumulator variable
        mutex_lock(&accumulator_mutex);
        // Check for overflow or underflow before performing the addition
        if ((value > 0 && accumulator > LONG_MAX - value) || (value < 0 && accumulator < LONG_MIN - value)) {
            pr_err("myaccumulator: Overflow or underflow detected. Accumulation aborted.\n");
            mutex_unlock(&accumulator_mutex); // Unlock the mutex
            kfree(kbuf);  // Free the kernel buffer
            return -EOVERFLOW;  // Return overflow error code
        }
        
        // Perform the addition if no overflow
        accumulator += value;
        mutex_unlock(&accumulator_mutex); // Unlock the mutex

        pr_info("myaccumulator: Value accumulated.\n");
    } else {
        kfree(kbuf);  // Free the kernel buffer
        pr_err("myaccumulator: Error converting string to long\n");
        return -EINVAL;  // Return error if conversion fails
    }

    kfree(kbuf); // Free the kernel buffer
    return len; // Return the number of bytes written
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};

/**
 * @brief Initialize the myaccumulator kernel module.
 *
 * This function allocates a character device region, initializes the character device,
 * creates the associated class and device in /dev/ and register some messages in the kernel log.
 *
 * @return 0 on success, negative error code on failure.
 */
static int __init myaccumulator_init(void) {

    // Allocate a device number, 0 means it is the first device, 1 means the amount of devices.
    if( alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0) {
        pr_err("myaccumulator: Failed to allocate device number\n");
        return -EBUSY; // Return error if allocation fails
    }else{
        pr_info("myaccumulator: Registered with major number: %d\n", MAJOR(dev_num));
    }

    // initialize the cdev structure
    cdev_init(&accumDevice, &fops);
    accumDevice.owner = THIS_MODULE; // Set the owner of the device to this module

    // Add the device to the system
    if (cdev_add(&accumDevice, dev_num, 1) < 0) {
        pr_err("myaccumulator: Failed to add device to system\n");
        unregister_chrdev_region(dev_num, 1); // Unregister the device number
        return -EINVAL; // Return error if adding fails
    }else{
        pr_info("myaccumulator: Device added to system\n");
    }

    // Create a class for the device
    accumClass = class_create(CLASS_NAME);
    if(IS_ERR(accumClass)) {
        pr_err("myaccumulator: Failed to create class\n");
        cdev_del(&accumDevice); // Delete the device
        unregister_chrdev_region(dev_num, 1); // Unregister the device number
        return PTR_ERR(accumClass); // Return error if class creation fails
    }else{
        pr_info("myaccumulator: Class created\n");
    }

    // Create a device 
    if (device_create(accumClass, NULL, dev_num, NULL, DEVICE_NAME) == NULL) {
        pr_err("myaccumulator: Failed to create device\n");
        class_destroy(accumClass); // Destroy the class
        cdev_del(&accumDevice); // Delete the device
        unregister_chrdev_region(dev_num, 1); // Unregister the device number
        return -ENODEV; // Return error if device creation fails
    }else{
        pr_info("myaccumulator: Device created\n");
    }
    
    return 0; 
}

/**
 * @brief Module cleanup function.
 *
 * This function is called when the module is removed from the kernel.
 * It performs cleanup by destroying the device, class, character device,
 * and unregistering the allocated device number.
 */
static void __exit myaccumulator_exit(void) {
    
    device_destroy(accumClass, dev_num); // Destroy the device
    class_destroy(accumClass); // Destroy the class
    cdev_del(&accumDevice); // Delete the device
    unregister_chrdev_region(dev_num, 1); // Unregister the device number

    pr_info("myaccumulator: Device removed\n");
}

// Register the initialization and exit functions
module_init(myaccumulator_init); // Register the initialization function
module_exit(myaccumulator_exit); // Register the exit function

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Franco Rodriguez");
MODULE_DESCRIPTION("A simple Linux kernel module to accumulate numbers");
MODULE_VERSION("1.0");
