#!/bin/bash

set -e
MODULE_PATH="build/myaccumulator.ko"
KEY_DIR="keys"
KEY_NAME="my_signing_key"
KEY_PRIV="${KEY_DIR}/${KEY_NAME}.key"
KEY_X509="${KEY_DIR}/${KEY_NAME}.crt"
KEY_DER="${KEY_DIR}/${KEY_NAME}.der"
SIGNED_MODULE_PATH="build/myaccumulator.ko"
DEVICE_NAME="myaccumulator"


print_help() {
    echo "Usage:"
    echo "  $0 --build                   Build the kernel module"
    echo "  $0 --sign                    Sign the kernel module"
    echo "  $0 --install                 Insert the kernel module"
    echo "  $0 --uninstall               Remove the kernel module"
    echo "  $0 --clean                   Clean build files"
    echo "  $0 --help                    Show this help message"
}

build_module() {
    # 1. Clean and build the project
    make clean
    make
}

sign_module() {
    mkdir -p "$KEY_DIR"

    # 1. Generate keys if they don't exist
    if [ ! -f "$KEY_PRIV" ] || [ ! -f "$KEY_X509" ]; then
        echo "[+] Generating keys..."
        openssl req -new -x509 -newkey rsa:2048 -keyout "$KEY_PRIV" -out "$KEY_X509" \
            -days 36500 -nodes -subj "/CN=MyKernelModule/"
    else
        echo "[*] Signing keys already exist."
    fi

    # 2. Sign the module
    echo "[+] Signing module..."
    sudo /usr/src/linux-headers-$(uname -r)/scripts/sign-file sha256 "$KEY_PRIV" "$KEY_X509" "$MODULE_PATH"

    # 3. Convert the certificate to DER
    echo "[+] Converting certificate to DER format..."
    openssl x509 -in "$KEY_X509" -outform DER -out "$KEY_DER"

    # 4. Copy the certificate to where mokutil expects it
    echo "[+] Copying DER certificate for MOK enrollment..."
    sudo cp "$KEY_DER" "/var/lib/shim-signed/mok/${KEY_NAME}.der"

    sudo mokutil --import /var/lib/shim-signed/mok/${KEY_NAME}.der

    # 5. Final instructions
    echo
    echo "[✔] Module signed successfully."
    echo
    echo "Reboot and follow the screen instructions to enroll the key."
    echo "After run "sudo ./install.sh --install" to insert the module."
}

install_module() {
    # 1. Check if already loaded
    if lsmod | grep -q "$DEVICE_NAME"; then
        echo "[*] Module already inserted. Removing it first..."
        sudo rmmod "$DEVICE_NAME"
    fi

    # 2. Insert module
    echo "[+] Inserting module..."
    sudo insmod "$MODULE_PATH"

    # 3. Check if it was inserted correctly
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

}

clean_module() {
    # 1. Clean the build files
    echo "[+] Cleaning build files..."
    make clean
    echo "[✔] Build files cleaned."
}

# --- Argument parsing ---
case "$1" in
    --clean)
        clean_module
        ;;
    --build)
        build_module "$4"
        ;;
    --sign)
        sign_module "$3"
        ;;
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

