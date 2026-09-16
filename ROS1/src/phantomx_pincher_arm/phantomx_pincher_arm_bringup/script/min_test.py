import serial
import time

PORT = "/dev/ttyUSB0"
BAUD = 9600

ser = serial.Serial(
    PORT,
    BAUD,
    timeout=None
)

time.sleep(2)

ser.reset_input_buffer()
ser.reset_output_buffer()

# ArbotiX ID = 253
# READ_DATA
# start register = 0
# read 2 bytes
packet = bytes([
    0xFF,
    0xFF,
    0xFD,
    0x04,
    0x02,
    0x00,
    0x02,
    0xFA
])

print("TX:", packet.hex(" "))

# Send COMPLETE packet first
ser.write(packet)
ser.flush()

# Give firmware a little time to process it
time.sleep(0.05)

# Now read the response
response = ser.read(8)

print("RX:", response.hex(" "))
print("RX bytes:", list(response))

ser.close()
