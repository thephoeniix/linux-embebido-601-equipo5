import sys
import glob

import serial#libreria pyserial 

def find_available_serial_ports() -> list[str]:
    if sys.platform.startswith('win'): # Computadora windows
        platform = 'win'
        ports =[f'COM{i}' for i in range(1, 256)]
    elif sys.platform.startswith('linux'): # Computadora Linux
        platform = 'linux'
        ports =glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'): # Mac
        platform = 'darwin'
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported Platform')
    result = []
    for port in ports:
        early_stop = True if platform == 'win' else False
        try:
            s= serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            if early_stop:
                break
            continue

    return result

#python -m venv env
#activar ambiente activate bat 
#
