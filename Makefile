# Nombre del módulo
obj-m := src/myaccumulator.o

# Directorios
BUILD_DIR := $(PWD)/build
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
SRC_DIR := $(shell pwd)/src

# Compilar el módulo
all:
	mkdir -p $(BUILD_DIR)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	mv $(SRC_DIR)/*.o $(SRC_DIR)/*.ko $(SRC_DIR)/*.mod.* $(SRC_DIR)/*.mod $(SRC_DIR)/.*.cmd $(BUILD_DIR)/

# Limpiar archivos generados
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf $(BUILD_DIR)
