# Linux character device
Development of a character device driver as part of the Eclypsium Internship Interview Challenge

## Theoretical Framework  
### Driver (Device Controller)

**Definition:** A driver is a software program that controls a hardware device. Drivers act as intermediaries between the operating system and the hardware, allowing software to access and control the hardware efficiently and safely.

**Function:** Its function is to manage the operations of the device and provide an interface for applications or the operating system to interact with the device.

### Linux Kernel Module

**Definition:** A Linux kernel module is precisely defined as a code segment capable of dynamic loading and unloading within the kernel as needed. These modules enhance kernel capabilities without necessitating a system reboot. The main diference is not all kernel modules are drivers, some modules add other functionalities to the kernel 

### Character Device Driver
**Definition:** A character device driver is a type of device driver that handles character devices, which allow data to be transferred to and from the device in continuous streams of bytes (characters). Unlike block devices, which manage data in fixed-size blocks, character devices deal with data transmission byte by byte. Common examples of character devices include terminals, printers, and serial ports.

### Major and Minor Numbers
The link between the APPLICATION and the CDF is based on the name of the device file. However, the link between the CDF and the DD is based on the device file number, not its name.
This means that a user-space application can refer to the CDF by any name, while in kernel space, the connection between CDF and CDD relies on a simple index-based link.

**Major Number:** This number identifies the device driver itself. Each driver is assigned a unique major number, which the kernel uses to redirect I/O operations to the appropriate driver.

**Minor Number:** This number identifies a specific device managed by a particular driver. In other words, while the major number points to the driver, the minor number identifies the specific device that the driver handles.

For example, if we have a disk driver with a major number of 8, the different disks or partitions managed by this driver might have minor numbers like 0, 1, 2, and so on.

We can use `ls -l /dev` or `lsmod` to see the nodes (whit their major and minor number) or the active kernel modules.

## Development

### Functional requirements

* The device should accept writes from user-space programs.
* Each write will be a string containing an integer number (positive or negative).
* The driver will parse the number and accumulate it into an internal counter.
* On a read() call, it should return the current accumulated value as a string.
* If the driver is uninstalled, the count should be reseted.

### Non-functional requirements

* Must create a device node in /dev/myaccumulator (or similar).
* Must implement the open, read, write, and release file operations.
* Should not leak memory.
* Should have unnit testing.

## Build

First, we have to install the header files for the kernel 

```bash
sudo apt-get update 
apt-cache search linux-headers-`uname -r`
```
Then we can use the following comand to install the headers

```bash
sudo apt-get install kmod linux-headers-[version]-generic
```

This kernel module contains the basic functions of each module. It includes the open and release functions to access the module, read and write functions to use it, and exit and init functions for installation.

You can use the `install.sh` script to make the whole process of using the module easier. To see the different usage options use the following commands in the console

```bash
sudo chmod +x install.sh
sudo ./install.sh --help
```
Since the build and mounting process is very different depending on whether the user has Secure Boot enabled or not, the install script has differents order for the commands

### Building using secure boot

First, we have to build the project

```bash
sudo ./install.sh --build
```
This generates the `build` directory where all the generated files are moved, including the module itself.

The kernel does not allow unsigned modules to be loaded. This is to prevent malicious drivers from sneaking into the system. To do this, you must generate a key pair and use the sign-file script to sign the module. The public part of the key must be imported into the system using the Machine Owner Key (MOK) framework.

```bash
sudo ./install.sh --sign
```
A `Keys` directory is generated to store your own key locally and signs the module. Then you have to reboot your pc to use the MOK administrator to upload your key to the system.

Finally yo use the following option to mount the module

```bash
sudo ./install.sh --install
```

### Building without Secure Boot

This is more easyer since we dont have to sign the module. The steps are the same without the need to sign the module and the reboot.

```bash
sudo ./install.sh --build
sudo ./install.sh --install
````
### After build

The scripts also has two more option to facilitate the unmounting and cleaning process

To unmount the kernell module run

```bash
sudo ./install.sh --uninstall
```
To clean the project after building run

```bash
sudo ./install.sh --clean
```

## Use the module

Now the module is installed and ready to be used. Whit the `dmesg` command you can see the kernel's message buffer and should be something like this

```bash
User@UserPC:~$ sudo dmesg | grep myaccumulator
[ 4276.234253] myaccumulator: Registered with major number: 506
[ 4276.234257] myaccumulator: Device added to system
[ 4276.234272] myaccumulator: Class created
[ 4276.234319] myaccumulator: Device created
```
You can interact with the module through the terminal with commands like `echo` whit tee/sh or `cat` directly.

```bash
User@UserPC:~$ sudo cat /dev/myaccumulator 
0
User@UserPC:~$ sudo sh -c 'echo 10 > /dev/myaccumulator'
User@UserPC:~$ echo 40 | sudo tee /dev/myaccumulator 
User@UserPC:~$ sudo cat /dev/myaccumulator 
50 
```
Or using the functions `open()`, `read()`, `write()` among others in a `.c` source code. For example

```C
int main() {
    int fd = open("/dev/myaccumulator", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char input[64], buffer[64];

    while (1) {
        printf("Enter number, 'read' or 'exit': ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        if (!strcmp(input, "exit")) break;

        if (!strcmp(input, "read")) {
            lseek(fd, 0, SEEK_SET);
            int n = read(fd, buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = 0;
                printf("Value: %s", buffer);
            } else {
                perror("read");
            }
        } else {
            strcat(input, "\n");
            if (write(fd, input, strlen(input)) < 0)
                perror("write");
        }
    }

    close(fd);
    return 0;
}
```
This should give the following result

```bash
User@UserPC:~/Desktop$ sudo ./driver_test.o 
Enter number, 'read' or 'exit': read
Value: 0
Enter number, 'read' or 'exit': 140
Written successfully.
Enter number, 'read' or 'exit': exit
User@UserPC:~/Desktop$ sudo ./driver_test.o 
Enter number, 'read' or 'exit': read
Value: 140
```
If you use again the `dmesg` command you can see everything you do.

```bash
[ 9210.255137] myaccumulator: Device opened - PID 10902, UID 0, comando: driver_test.o
[ 9255.462578] myaccumulator: Device opened - PID 10918, UID 0, comando: sh
[ 9255.462591] myaccumulator: Value accumulated
[ 9255.462594] myaccumulator: Device closed by PID 10918
[ 9258.754772] myaccumulator: Device opened - PID 10926, UID 0, comando: cat
[ 9258.754790] myaccumulator: Read 0 from device
[ 9258.754810] myaccumulator: Device closed by PID 10926
[ 9263.399282] myaccumulator: Read 0 from device
[ 9266.296044] myaccumulator: Value accumulated
[ 9268.677261] myaccumulator: Device closed by PID 10902
[ 9269.914452] myaccumulator: Device opened - PID 10936, UID 0, comando: driver_test.o
[ 9274.370056] myaccumulator: Read 140 from device
[ 9364.097250] myaccumulator: Device closed by PID 10936
```
## Memory Leak

To ensure this linux kernel module is free of memory leaks, I set up a test environment by building a secondary Linux kernel with the configuration option `CONFIG_DEBUG_KMEMLEAK=y`. This option enables the `kmemleak` subsystem, kmemleak is a lightweight memory leak detector that works by scanning kernel memory and tracking unreachable allocations over time.

The system was emulated with QEMU using the `6.8.4` linux kernel's version , allowing full control over kernel boot parameters and module testing without affecting the host environment. Once booted, I loaded my original kernel module ( the same one that was built in the host system) into the test kernel and began interacting with it from user space using its character device interface `/dev/myaccumulator`.

Test for leaks:

* Mount the original driver and use it like always with `cat` and `echo` commands. After a while you can see it doesnt́ have memory leaks repported.

<div align="center">
  <img src="https://github.com/user-attachments/assets/47829127-8c12-46d9-a60d-0f8e2d332b24" alt="memleak first test" width="70%">
</div>

* Deliberately introduced memory leaks in `read()` and `write()` function of the module by allocating memory with `kmalloc()` but without free it, simulating a leak. Whit the same process of the original we get:

<div align="center">
  <img src="https://github.com/user-attachments/assets/76b9ada2-83bf-400f-9263-19607f9007c7" alt="memleak first test" width="70%">
</div>

This entry shows a `1024-byte` allocation made by a `cat` command calling `dev_read()` function, which was never freed, resulting in a memory leak.

## Testing

We can test the module in 2 ways:
* User-level 
* Kernel-level

For the first one you can find in the `test/user-level` directory a small pyhton script which uses `unittest` python module as a test suite. run the following command to perform it

```bash
cd test/user-level
python3 test.py
```
If you want a more advanced testing you can use `KUnit`. KUnit is the official unit testing framework for the Linux kernel. It allows writing and running tests directly within the kernel, in isolation and without relying on hardware or user-space tools.

In the `test/kernel-level` directory you can find a `setup_enviroment.md` with instructions for build the `6.8.4` version of the Linux kernel, how to activate the necessary flags to use kunit and kmemleak and be able to emulate it with qemu.




















 









