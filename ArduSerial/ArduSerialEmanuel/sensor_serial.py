import time
import serial
class SensorSerial:
    def __init__(self, 
                    serial_port:str,
                    baudrate: int = 115200,
                    timeout: float = 2.0,
                    connection_time: float = 3.0,
                    reception_time: float = 0.5
                ) -> None:
                self.serial_connection = serial.Serial(
                    port=serial_port,
                    timeout=timeout,
                    baudrate=baudrate
                )
                self.connection_time =connection_time
                self.reception_time = reception_time
                time.sleep(self.connection_time)

    def send(self, to_send:str)->str | None:
            self.serial_connection.write(to_send.encde("utf-8"))
            time.sleep(self.reception_time)
            recived = self.serial_connection.readline()
            return recived
    
    def recive(self, )->str|None:
            recived = self.serial_connection.readline()
            return recived
    
    def close(self):
            self.serial_connection.close()

    def __str__(self) -> None:
            return f"SerialSensor({self.serial_connection}, {self.connection_time=}, {self.reception_time=})"

    def __repr__(self) -> None:
            pass
        