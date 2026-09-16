#!/usr/bin/env python3

import serial
import time

PORT = "/dev/ttyUSB0"
BAUD = 9600

# Read 2 bytes starting at register 0 from ArbotiX ID 253
packet = bytes([
    0xFF,
    0xFF,
    0xFD,   # ArbotiX ID = 253
    0x04,   # length
    0x02,   # AX_READ_DATA
    0x00,   # starting register
    0x02,   # read two bytes
    0xFA    # checksum
])

ser = serial.Serial(
    port=PORT,
    baudrate=BAUD,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=2.0,
    xonxoff=False,
    rtscts=False,
    dsrdtr=False
)

# Opening the port may reset the ArbotiX through DTR.
time.sleep(3.0)

ser.reset_input_buffer()

print("Connected to", PORT, "at", BAUD, "baud")
print("TX:", packet.hex(" "))

ser.write(packet)
ser.flush()

time.sleep(0.5)

print("Bytes waiting:", ser.in_waiting)

response = ser.read(100)

print("RX raw:", response.hex(" "))

if response:
    print("RX bytes:", list(response))
else:
    print("No response received")

ser.close()
