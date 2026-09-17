import argparse
import serial
import sys
import time

PORT = "/dev/ttyUSB0"
BAUD = 9600

def menu():
    parser = argparse.ArgumentParser(description="Command a Dynamixel servo through an ArbotiX controller.")
    parser.add_argument("servo_id", type=int, choices=range(1, 254), help="Servo ID, integer from 1 to 253.")
    parser.add_argument("position", type=int, choices=range(0, 1024), help="Target position, integer from 0 to 1023.")

    return parser.parse_args()

def checksum(data):
    return 255 - (sum(data) % 256)


def write_position(ser, servo_id, position):
    # AX-12 Goal Position register
    address = 30

    low = position & 0xFF
    high = (position >> 8) & 0xFF

    # Dynamixel Protocol 1.0 WRITE_DATA
    # ID, LENGTH, INSTRUCTION, ADDRESS, LOW, HIGH
    body = [servo_id, 5, 0x03, address, low, high]

    packet = bytes([0xFF, 0xFF] + body + [checksum(body)])

    ser.reset_input_buffer()

    print("TX:", packet.hex(" "))

    ser.write(packet)
    ser.flush()

    # Wait until the complete command has been transmitted
    time.sleep(0.05)

    # ros.ino sends a status packet after WRITE_DATA
    response = ser.read(6)

    if len(response) != 6:
        return None

    if response[0] != 0xFF or response[1] != 0xFF:
        return None

    if response[2] != servo_id:
        return None

    return response


def main():
    args = menu()

    servo_id = args.servo_id
    position = args.position

    print(f"Servo ID: {servo_id}")
    print(f"Position: {position}")

    ser = serial.Serial(PORT, BAUD, timeout=2)

    time.sleep(2)

    print(f"Servo ID : {servo_id}")
    print(f"Position : {position}")

    trials = 3
    for i in range(trials):
        response = write_position(ser, servo_id, position)

        if response:
            print("RX:", response.hex(" "))

            error = response[4]

            if error == 0:
                print("Command accepted.")
                break
            else:
                print(f"Servo returned error code: {error}. Retrying...")
        else:
            print("No valid response received. Retrying...")

    ser.close()


if __name__ == "__main__":
    main()
