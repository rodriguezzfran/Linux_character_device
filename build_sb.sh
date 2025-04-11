#!/bin/bash

set -e  # Exit on error

# Variables
MODULE_PATH="build/myaccumulator.ko"
KEY_DIR="keys"
KEY_NAME="my_signing_key"
KEY_PRIV="${KEY_DIR}/${KEY_NAME}.key"
KEY_X509="${KEY_DIR}/${KEY_NAME}.crt"
KEY_DER="${KEY_DIR}/${KEY_NAME}.der"
SIGNED_MODULE_PATH="build/myaccumulator.ko"

# Crear carpeta de claves si no existe
make clean
make
mkdir -p "$KEY_DIR"

# 1. Generar claves si no existen
if [ ! -f "$KEY_PRIV" ] || [ ! -f "$KEY_X509" ]; then
    echo "[+] Generating keys..."
    openssl req -new -x509 -newkey rsa:2048 -keyout "$KEY_PRIV" -out "$KEY_X509" \
        -days 36500 -nodes -subj "/CN=MyKernelModule/"
else
    echo "[*] Signing keys already exist."
fi

# 2. Firmar el módulo
echo "[+] Signing module..."
sudo /usr/src/linux-headers-$(uname -r)/scripts/sign-file sha256 "$KEY_PRIV" "$KEY_X509" "$MODULE_PATH"

# 3. Convertir el certificado a DER
echo "[+] Converting certificate to DER format..."
openssl x509 -in "$KEY_X509" -outform DER -out "$KEY_DER"

# 4. Copiar el certificado a donde mokutil lo espera
echo "[+] Copying DER certificate for MOK enrollment..."
sudo cp "$KEY_DER" "/var/lib/shim-signed/mok/${KEY_NAME}.der"

sudo mokutil --import /var/lib/shim-signed/mok/${KEY_NAME}.der

# 5. Instrucciones finales
echo
echo "[✔] Module signed successfully."
echo
echo "Reboot and follow the instructions to complete MOK enrollment."
echo "After that, you will be able to insert the module with:"
echo "  sudo insmod $SIGNED_MODULE_PATH"



