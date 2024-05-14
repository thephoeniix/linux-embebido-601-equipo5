import os
def find_available_serial_ports() -> list[str]:
    dev_files = os.listdir('/dev/ttyA*/')
    return dev_files
print(find_available_serial_ports())