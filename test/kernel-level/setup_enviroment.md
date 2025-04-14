### Update and get the dependencies
```bash
sudo apt update
sudo apt install build-essential flex bison libssl-dev libelf-dev bc qemu-system-x86
```
### Install the required packages for the kernel build

```bash
mkdir -p ~/kmemleak-qemu && cd ~/kmemleak-qemu
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.8.4.tar.xz
tar -xvf linux-6.8.4.tar.xz
cd linux-6.8.4
make defconfig
make menuconfig
```
Here you have to activate the following options (use / for search)

* CONFIG_DEBUG_KERNEL → Activate
* CONFIG_DEBUG_KMEMLEAK → Activate
* CONFIG_DEBUG_FS → Activate
* CONFIG_KALLSYMS → Activate
* CONFIG_KUNIT → Activate
* CONFIG_MYACCUMULATOR → Activate
* CONFIG_MYACCUMULATOR_KUNIT_TEST → Activate

```bash
make -j$(nproc)
cd ~/kmemleak-qemu
```
### Install the busybox

```bash
wget https://busybox.net/downloads/busybox-1.36.1.tar.bz2
tar -xjf busybox-1.36.1.tar.bz2
cd busybox-1.36.1
make defconfig
make menuconfig
```
Here also you have to Busybox Settings -> Settings -> Build Busybox as a static binary (no shared libs)
Maybe you should deactivate the following options: Networking support

```bash
make -j$(nproc)
```

### Generate the initramfs and the kernel structure

```bash
make CONFIG_PREFIX=../initramfs install
cd ../initramfs
mkdir -p proc sys dev etc mnt tmp var lib/modules root
sudo mknod dev/console c 5 1
sudo mknod dev/null c 1 3
```

#### Init script that will be executed by the kernel

```bash
cat << 'EOF' > init
#!/bin/sh

mount -t proc none /proc
mount -t sysfs none /sys
mount -t debugfs none /sys/kernel/debug

echo "Running init..."

# Acá podés probar insmod / ver kmemleak
# por ejemplo: insmod /root/myaccumulator.ko

# Activar kmemleak
echo scan > /sys/kernel/debug/kmemleak
echo clear > /sys/kernel/debug/kmemleak

exec /bin/sh
EOF

chmod +x init

# Create the initramfs image
```bash
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.cpio.gz
```

### Create a shared directory to be able to share files between the host and the guest

```bash
mkdir -p ~/shared-kernel-modules
```
In the VM you can mount it with:

```bash
mkdir /mnt/host
mount -t 9p -o trans=virtio hostshare /mnt/host
```

### init qemu

```bash
qemu-system-x86_64 \
  -kernel ./linux-6.8.4/arch/x86/boot/bzImage \
  -initrd ./initramfs.cpio.gz \
  -m 512M \
  -nographic \
  -append "console=ttyS0 root=/dev/ram rdinit=/init" \
  -fsdev local,id=fsdev0,path=/home/franco/shared-kernel-modules,security_model=none \
  -device virtio-9p-pci,fsdev=fsdev0,mount_tag=hostshare
```








