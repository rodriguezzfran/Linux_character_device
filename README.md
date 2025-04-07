# Linux_character_device
Development of a character device driver as part of the Eclypsium Internship Interview Challenge

## Theoretical Framework  
### Driver (Device Controller)

**Definition:** A driver is a software program that controls a hardware device. Drivers act as intermediaries between the operating system and the hardware, allowing software to access and control the hardware efficiently and safely.

**Function:** Its function is to manage the operations of the device and provide an interface for applications or the operating system to interact with the device.

### Linux Kernel Module

**Definition:** A Linux kernel module is precisely defined as a code segment capable of dynamic loading and unloading within the kernel as needed. These modules enhance kernel capabilities without necessitating a system reboot. The main diference is not all kernel modules are drivers, some modules add other functionalities to the kernel 

### Character Device Driver
**Definition:** A character device driver is a type of device driver that handles character devices, which allow data to be transferred to and from the device in continuous streams of bytes (characters). Unlike block devices, which manage data in fixed-size blocks, character devices deal with data transmission byte by byte. Common examples of character devices include terminals, printers, and serial ports.



