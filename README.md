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


 









