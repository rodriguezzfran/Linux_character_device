#!/bin/bash

set -e

DEVICE_NAME="myaccumulator"
MODULE_PATH="build/myaccumulator.ko"
SIGNED_MODULE_PATH="build/myaccumulator.ko"

print_help() {
    echo "Usage:"
    echo "  $0 --install                 Insert the kernel module"
    echo "  $0 --uninstall               Remove the kernel module"
    echo "  $0 --help                    Show this help message"
}

install_module() {
    # 1. Clean and build the project
    make clean
    make

    # 1. Verificar si ya está cargado
    if lsmod | grep -q "$DEVICE_NAME"; then
        echo "[*] Module already inserted. Removing it first..."
        sudo rmmod "$DEVICE_NAME"
    fi

    # 2. Insertar módulo
    echo "[+] Inserting module..."
    sudo insmod "$MODULE_PATH"

    # 3. Verificar si se insertó correctamente
    if lsmod | grep -q "$DEVICE_NAME"; then
        echo "[✔] Module inserted successfully."
    else
        echo "[✘] Failed to insert module."
        exit 1
    fi
}

uninstall_module() {
    # Verificar si está cargado
    if lsmod | grep -q "$DEVICE_NAME"; then
        echo "[+] Removing module..."
        sudo rmmod "$DEVICE_NAME"
        echo "[✔] Module removed successfully."
    else
        echo "[✘] Module is not loaded."
    fi

    make clean
    echo "[✔] Cleaned up build files."
    echo "[✔] Module uninstalled successfully."
}

# --- Argument parsing ---
case "$1" in
    --install)
        install_module "$2"
        ;;
    --uninstall)
        uninstall_module
        ;;
    --help | *)
        print_help
        ;;
esac


