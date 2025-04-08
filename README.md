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

<div align="center">
  <img src="https://github.com/user-attachments/assets/1fef1c4b-ca28-4bdd-86f3-e5fa377559b1" alt="FPGA Diagram" style="box-shadow: 10px 10px 20px rgba(0, 0, 0, 0.5);">
</div>

<div align="center">
  <img src="https://github.com/user-attachments/assets/b4297899-2bcf-47da-a937-5d4cb5c54470" alt="FPGA Diagram" style="box-shadow: 10px 10px 20px rgba(0, 0, 0, 0.5);">
</div>

## Development

First, we have to install the header files for the kernel 

```bash
sudo apt-get update 
apt-cache search linux-headers-`uname -r`
```
Then we can use the following comand to install the headers

```bash
sudo apt-get install kmod linux-headers-[version]-generic
```
<div align="center">
  <img src="https://github.com/user-attachments/assets/87640edd-3142-48a1-b04d-d16f2fe47ef0" alt="FPGA Diagram" style="box-shadow: 10px 10px 20px rgba(0, 0, 0, 0.5);">
</div>

 









