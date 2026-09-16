import serial
import time

PORT = "/dev/ttyUSB0"
BAUD = 9600

ser = serial.Serial(
    PORT,
    BAUD,
    timeout=2
)

time.sleep(2)

ser.reset_input_buffer()
ser.reset_output_buffer()

# AX-12 ID 1
# READ_DATA
# Present Position starts at register 36
# Read 2 bytes
packet = bytes([
    0xFF,
    0xFF,
    0x01,
    0x04,
    0x02,
    0x24,
    0x02,
    0xD2
])

print("TX:", packet.hex(" "))

ser.write(packet)
ser.flush()

time.sleep(0.05)

response = ser.read(8)

print("RX:", response.hex(" "))
print("RX bytes:", list(response))

if len(response) >= 8:
    low = response[5]
    high = response[6]

    position = low + (high << 8)

    print("Servo 1 position:", position)

ser.close()