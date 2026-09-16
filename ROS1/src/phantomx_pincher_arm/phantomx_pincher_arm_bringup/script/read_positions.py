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

trials = 3
for servo_id in range(1, 6):
    for i in range(trials):
        response = read_register(ser, servo_id, address=36, length=2)
        if response:
            print(f"Response received with size: {len(response)} bytes")
            break
        else:
            print("No valid response. Retrying...")
        time.sleep(0.1)

    if response:
        position = response[5] | (response[6] << 8)
        print(f"Servo {servo_id}: {position}")

    time.sleep(0.1)

ser.close()



