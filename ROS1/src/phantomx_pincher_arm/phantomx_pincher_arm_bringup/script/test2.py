import serial
import time

ser = serial.Serial("/dev/ttyUSB0", 9600, timeout=2)

time.sleep(2)

ser.reset_input_buffer()
ser.reset_output_buffer()

packet = bytes([
    0xFF, 0xFF,
    0xFD,       # ArbotiX ID
    0x04,
    0x02,       # READ
    0x00,
    0x02,
    0xFA
])

print("TX:", packet.hex(" "))

ser.write(packet)
ser.flush()

time.sleep(0.05)

response = ser.read(8)

print("RX:", response.hex(" "))

ser.close()
