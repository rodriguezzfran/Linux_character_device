#!/usr/bin/env python3
import unittest
import os
import stat
import subprocess
import sys

DEVICE_PATH = "/dev/myaccumulator"

# Check if the module is loaded
def is_module_loaded(module_name):
    try:
        output = subprocess.check_output(["lsmod"]).decode()
        return module_name in output
    except Exception:
        return False

# Check if the device is present and is a character device
def is_device_present(path):
    return os.path.exists(path) and stat.S_ISCHR(os.stat(path).st_mode)

# Check if the device has read/write permissions
def has_rw_permissions(path):
    return os.access(path, os.R_OK | os.W_OK)

# Check if the script is running as root
def is_root():
    return os.geteuid() == 0

# To reset the device after the tests
def reset_device(self):
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
        with open(DEVICE_PATH, "w") as dev:
            dev.write(f"{-value}\n")

# Everything else
class TestMyAccumulator(unittest.TestCase):

    def setUp(self):
        """Reset the accumulator before each test"""
        try:
            with open(DEVICE_PATH, "r") as dev:
                value = int(dev.read().strip())
            with open(DEVICE_PATH, "w") as dev:
                dev.write(f"{-value}\n")
        except Exception as e:
            self.fail(f"Failed to reset accumulator before test: {e}")

    def test_01_initial_value_is_zero(self):
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
        if value != 0:
            raise AssertionError(f" Value is not 0: {value}")
        self.assertEqual(value, 0)
        print("[+] test initial_value_is_zero passed")
        sys.stdout.flush()

    def test_02_write_value(self):
        with open(DEVICE_PATH, "w") as dev:
            dev.write("50\n")
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
        if value != 50:
            raise AssertionError(f" Value is not 50: {value}")
        self.assertEqual(value, 50)
        print("[+] test write_value passed")
        sys.stdout.flush()
    
    def test_03_accumulate_multiple_writes(self):
        with open(DEVICE_PATH, "w") as dev:
            dev.write("10\n")
        with open(DEVICE_PATH, "w") as dev:
            dev.write("20\n")
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
            if value != 30:
                raise AssertionError(f" Value is not the expected sum: {value}")
        self.assertEqual(value, 30)
        print("[+] test accumulate_multiple_writes passed")
        sys.stdout.flush()

    def test_04_negative_write_decreases_accumulator(self):
        with open(DEVICE_PATH, "w") as dev:
            dev.write("100\n")
        with open(DEVICE_PATH, "w") as dev:
            dev.write("-30\n")
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
            if value != 70:
                raise AssertionError(f" Value is not the expected sum: {value}")
        self.assertEqual(value, 70)
        print("[+] test negative_write_decreases_accumulator passed")
        sys.stdout.flush()
    
    def test_05_invalid_input_is_ignored_or_fails(self):
        with open(DEVICE_PATH, "w") as dev:
            dev.write("50\n")
        try:
            with open(DEVICE_PATH, "w") as dev:
                dev.write("notanumber\n")
        except Exception as e:
            self.assertIsInstance(e, (ValueError, OSError, IOError))
        # Check it didn't change
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
            if value != 50:
                raise AssertionError(f" Value was not modified: {value}")
        self.assertEqual(value, 50)
        print("[+] test invalid_input_is_ignored_or_fails passed")
        sys.stdout.flush()
    
    def test_overflow_handling(self):

        # Define a very close to the maximum value for a 64-bit signed integer
        max_value = 9223372036854775806
        overflow_value = max_value + 1

        # try to write the maximum value
        with open(DEVICE_PATH, "w") as dev:
            dev.write("9223372036854775806\n")  # max value for 64-bit signed int

        # Test that the value is set correctly
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
        
        self.assertEqual(value, max_value)

        # Now overflow
        try:
            with open(DEVICE_PATH, "w") as dev:
                dev.write(f"{overflow_value}\n")
        except Exception as e:
            self.assertIsInstance(e, (ValueError, OSError, IOError))
        
        # Check that the value is still the max value
        with open(DEVICE_PATH, "r") as dev:
            value = int(dev.read().strip())
            if value == overflow_value:
                raise AssertionError(f" Value was not modified: {value}")
        self.assertEqual(value, max_value)
        print("[+] test overflow_handling passed")
        sys.stdout.flush()
  
if __name__ == "__main__":
    print("Checking environment...")

    if not is_root():
        print("[x] This script must be run as root. Use `sudo`.")
        sys.exit(1)

    if not is_device_present(DEVICE_PATH):
        print(f"[x] Device {DEVICE_PATH} not found or is not a character device.")
        sys.exit(1)

    if not has_rw_permissions(DEVICE_PATH):
        print(f"[x] You do not have read/write access to {DEVICE_PATH}.")
        sys.exit(1)

    if not is_module_loaded("myaccumulator"):
        print("[x] Module 'myaccumulator' does not loaded.")

    print("[+] Environment OK. Running tests...\n")
    unittest.main()
    print("\n[+] Tests completed.")
    reset_device()
    
