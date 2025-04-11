# Module name
obj-m := src/myaccumulator.o

# Paths
BUILD_DIR := $(PWD)/build
KDIR := /lib/modules/$(shell uname -r)/build
KEYS_DIR := $(PWD)/keys
PWD := $(shell pwd)
SRC_DIR := $(shell pwd)/src

# Compile and move to build directory
all:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	mv $(SRC_DIR)/*.ko $(SRC_DIR)/*.mod.c $(SRC_DIR)/*.mod $(SRC_DIR)/*.o $(SRC_DIR)/.*.cmd $(BUILD_DIR)/
	

# Clean up
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf $(KEYS_DIR)
	rm -rf $(BUILD_DIR)
