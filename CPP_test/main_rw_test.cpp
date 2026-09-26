#include "arbotix_driver.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

int main()
{
    const std::string port = "/dev/ttyUSB0";
    const int baud_rate = 9600;
    const uint8_t servo_id = 1;

    phantomx_pincher_hardware::ArbotixDriver driver;

    if (!driver.open(port, baud_rate))
    {
        std::cerr << "Failed to open serial port." << std::endl;
        return 1;
    }

    driver.wakeUpSerial();

    // ----------------------------------------------------
    // 1. Read initial position
    // ----------------------------------------------------

    uint16_t initial_position = 0;

    if (!driver.read_position(servo_id, initial_position))
    {
        std::cerr << "Failed to read initial position." << std::endl;
        return 1;
    }

    std::cout
        << "Initial present position: "
        << initial_position
        << std::endl;

    // ----------------------------------------------------
    // 2. Enable torque
    // ----------------------------------------------------

    if (!driver.enable_torque(servo_id, true))
    {
        std::cerr << "Failed to send torque-enable command." << std::endl;
        return 1;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(50));

    // ----------------------------------------------------
    // 3. READ BACK torque-enable register
    // ----------------------------------------------------

    std::vector<uint8_t> torque_data;

    if (driver.read_register(
            servo_id,
            24,
            1,
            torque_data))
    {
        std::cout
            << "Torque Enable register = "
            << static_cast<int>(torque_data[0])
            << std::endl;
    }
    else
    {
        std::cerr
            << "Could not read Torque Enable register."
            << std::endl;
    }

    // ----------------------------------------------------
    // 4. Write new goal position
    // ----------------------------------------------------

    const uint16_t target_position =
        initial_position + 20;

    std::cout
        << "Writing Goal Position = "
        << target_position
        << std::endl;

    if (!driver.write_position(
            servo_id,
            target_position))
    {
        std::cerr
            << "Failed to send Goal Position."
            << std::endl;

        return 1;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(50));

    // ----------------------------------------------------
    // 5. READ BACK Goal Position register
    // ----------------------------------------------------

    std::vector<uint8_t> goal_data;

    if (driver.read_register(
            servo_id,
            30,
            2,
            goal_data))
    {
        if (goal_data.size() >= 2)
        {
            const uint16_t goal_position =
                static_cast<uint16_t>(goal_data[0]) |
                (
                    static_cast<uint16_t>(goal_data[1])
                    << 8
                );

            std::cout
                << "Goal Position register = "
                << goal_position
                << std::endl;
        }
    }
    else
    {
        std::cerr
            << "Could not read Goal Position register."
            << std::endl;
    }

    // ----------------------------------------------------
    // 6. Wait and read actual position
    // ----------------------------------------------------

    std::this_thread::sleep_for(
        std::chrono::seconds(1));

    uint16_t measured_position = 0;

    if (driver.read_position(
            servo_id,
            measured_position))
    {
        std::cout
            << "Present Position after command = "
            << measured_position
            << std::endl;
    }

    driver.close();

    return 0;
}