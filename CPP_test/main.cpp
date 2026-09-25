#include "arbotix_driver.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    const std::string port = "/dev/ttyUSB0";
    const int baud_rate = 9600;

    phantomx_pincher_hardware::ArbotixDriver driver;

    if (!driver.open(port, baud_rate)) {
        std::cerr << "Could not open serial port " << port << std::endl;
        return 1;
    }

    std::cout << "Scanning Dynamixel IDs..." << std::endl;

    const int trials = 3;

    for (uint8_t servo_id = 1; servo_id <= 5; ++servo_id) {
        bool response_received = false;
        std::vector<uint8_t> data;

        for (int i = 0; i < trials; ++i) {
            data.clear();

            /*
             * Equivalent to:
             *
             * read_register(
             *     ser,
             *     servo_id,
             *     address=36,
             *     length=2)
             */
            response_received = driver.read_register(servo_id, 36, 2, data);

            if (response_received) {
                std::cout << "Response received with size: " << data.size() << " parameter bytes" << std::endl;
                break;
            }

            std::cout << "No valid response. Retrying..." << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (response_received) {
            if (data.size() >= 2) {
                /*
                 * Python:
                 *
                 * position =
                 *     response[5] |
                 *     (response[6] << 8)
                 *
                 * Our driver strips the Dynamixel packet header,
                 * ID, LENGTH, ERROR and CHECKSUM.
                 *
                 * Therefore:
                 *
                 * data[0] = POS_L
                 * data[1] = POS_H
                 */
                const uint16_t position = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
                std::cout << "Servo " << static_cast<int>(servo_id) << ": " << position << std::endl;

            } else {
                std::cerr << "Invalid response size for servo " << static_cast<int>(servo_id) << std::endl;
            }
        }
    }

    driver.close();

    return 0;
}