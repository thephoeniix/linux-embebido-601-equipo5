import os

def find_available_serial_ports() -> list[str]:
    dev_files = os.listdir('/dev/')  # Obtener la lista de todos los archivos en /dev
    serial_ports = []
    for file in dev_files:
        if file.startswith('ttyA'):
            serial_ports.append('/dev/' + file)
    return serial_ports

serial = find_available_serial_ports()

i = 0
while i < len(serial):
    print(serial[i])
    i += 1
