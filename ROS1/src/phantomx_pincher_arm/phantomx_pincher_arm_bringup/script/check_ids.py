import serial
import time

PORT = "/dev/ttyUSB0"
BAUD = 9600

def checksum(data):
    return 255 - (sum(data) % 256)

def read_register(ser, servo_id, address, length):
    # Dynamixel Protocol 1.0 READ_DATA
    body = [servo_id, 4, 0x02, address, length]

    packet = bytes([0xFF, 0xFF] + body + [checksum(body)])
    ser.reset_input_buffer()

    ser.write(packet)
    ser.flush()

    # Important with your setup: wait until TX is complete
    # before trying to receive the response.
    time.sleep(0.02)

    response = ser.read(length + 6)

    print(f"Response received with size: {len(response)} bytes")
    if len(response) < length + 6:
        return None

    if response[0] != 0xFF or response[1] != 0xFF:
        return None

    if response[2] != servo_id:
        return None

    return response


ser = serial.Serial(PORT, BAUD, timeout=5)
time.sleep(2)

print("Scanning Dynamixel IDs...")

found = []

# Dynamixel IDs normally range from 0 to 253.
# 254 is broadcast, so don't query it.
for servo_id in range(0, 6):

    response = read_register(
        ser,
        servo_id,
        address=3,
        length=1
    )

    if response is not None:
        print(f"Found servo ID {servo_id}")
        found.append(servo_id)

    else:
        print("Servo not found")

    time.sleep(0.1)

print()
print("Found IDs:", found)

ser.close()
