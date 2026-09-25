#include "phantomx_pincher_hardware/arbotix_driver.hpp"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include <fcntl.h>
#include <unistd.h>

namespace phantomx_pincher_hardware
{

ArbotixDriver::ArbotixDriver() : serial_fd_(-1) {
    speedsMap_[9600] = B9600;
    speedsMap_[19200] = B19200;
    speedsMap_[38400] = B38400;
    speedsMap_[57600] = B57600;
    speedsMap_[115200] = B115200;
}


ArbotixDriver::~ArbotixDriver() {
    this->close();
}


bool ArbotixDriver::open(const std::string& port, int baud_rate) {
    this->close();

    port_ = port;

    serial_fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY);

    if (serial_fd_ < 0) {
        std::cerr
            << "[ArbotixDriver] ERROR: Could not open serial port '"
            << port << "': " << std::strerror(errno) << std::endl;

        return false;
    }

    if (!configure_port(baud_rate)) {
        this->close();
        return false;
    }

    /*
     * Required by this ArbotiX setup.
     * Same delay as the working Python test.
     */
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Clear pending RX data.
    if (tcflush(serial_fd_, TCIFLUSH) != 0) {
        std::cerr << "[ArbotixDriver] WARNING: tcflush failed: " << std::strerror(errno) << std::endl;
    }

    wakeUpSerial();
    std::cout << "[ArbotixDriver] Opened serial port " << port << " at " << baud_rate << " baud" << std::endl;

    return true;
}

void ArbotixDriver::wakeUpSerial() {
    /// For some reason, the system needs a first package to wake up.
    // In order to add external logic outside, we do it during initialization.
    // Without this message, the first package is always lost.
    const uint8_t servo_id = 1;
    const uint8_t packet_length = 4;
    const uint8_t instruction = 0x02;
    const uint8_t address = 36;
    const uint8_t length = 2;

    std::vector<uint8_t> body{servo_id, packet_length, instruction, address, length};
    std::vector<uint8_t> packet{0xFF, 0xFF};

    packet.insert(packet.end(), body.begin(), body.end());
    packet.emplace_back(checksum(body));

    // Discard anything already present in RX.
    tcflush(serial_fd_, TCIFLUSH);

    // Send the dummy packet.
    write_bytes(packet);

    // Give the serial interface time to process it.
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // Discard any possible response.
    tcflush(serial_fd_, TCIFLUSH);
}

void ArbotixDriver::close() {
    if (serial_fd_ >= 0) {
        ::close(serial_fd_);
        serial_fd_ = -1;

        std::cout << "[ArbotixDriver] Closed serial port '" << port_ << "'" << std::endl;
    }
}


bool ArbotixDriver::is_open() const {
    return serial_fd_ >= 0;
}


bool ArbotixDriver::configure_port(int baud_rate) {
    struct termios tty;
    struct termios tty_old;

    std::memset(&tty, 0, sizeof tty);

    // Error handling.
    if (tcgetattr(serial_fd_, &tty) != 0) {
        std::cerr << "[ArbotixDriver] ERROR: tcgetattr failed: "
            << std::strerror(errno) << std::endl;
        return false;
    }

    // Save old tty parameters.
    tty_old = tty;

    // Get requested baud rate.
    speed_t speed;

    if (speedsMap_.count(baud_rate)) {
        speed = speedsMap_[baud_rate];
    } else {
        std::cerr << "[ArbotixDriver] ERROR: Unsupported baud rate: " << baud_rate << std::endl;
        return false;
    }

    // Set baud rate.
    if (cfsetospeed(&tty, speed) != 0 || cfsetispeed(&tty, speed) != 0) {
        std::cerr
            << "[ArbotixDriver] ERROR: Unable to set baud rate: "
            << std::strerror(errno)
            << std::endl;

        return false;
    }

    // Raw mode.
    cfmakeraw(&tty);

    /*
     * Port configuration:
     *
     * 8 data bits
     * no parity
     * 1 stop bit
     */
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    // No hardware flow control.
    tty.c_cflag &= ~CRTSCTS;

    // Enable receiver and ignore modem control lines.
    tty.c_cflag |= CREAD | CLOCAL;


    /*
     * Read timeout.
     *
     * VMIN = 0:
     * read() does not require any minimum number of bytes.
     *
     * VTIME = 50:
     * timeout = 5 seconds.
     *
     * VTIME is expressed in tenths of a second.
     */
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;

    // Clear the input buffer before applying settings.
    if (tcflush(serial_fd_, TCIFLUSH) != 0) {
        std::cerr << "[ArbotixDriver] WARNING: tcflush failed: " << std::strerror(errno) << std::endl;
        return false;
    }

    // Apply serial configuration.
    if (tcsetattr(serial_fd_, TCSANOW, &tty) != 0) {
        std::cerr << "[ArbotixDriver] ERROR: tcsetattr failed: " << std::strerror(errno) << std::endl;
        return false;
    }

    return true;
}


bool ArbotixDriver::write_bytes(const std::vector<uint8_t>& data) {
    if (!is_open()) {
        std::cerr << "[ArbotixDriver] ERROR: Cannot write: " << "serial port is not open" << std::endl;
        return false;
    }

    int n_written = 0;
    std::size_t i = 0;

    do {
        n_written = ::write(serial_fd_, &data[i], data.size() - n_written);
        if (n_written > 0) {
            i += static_cast<std::size_t>(n_written);
        }
    } while (data.size() > i && n_written > 0);

    if (n_written < 0) {
        std::cerr << "[ArbotixDriver] ERROR: Serial write failed: " << std::strerror(errno)<< std::endl;
        return false;
    }

    // Flush the port.
    if (tcdrain(serial_fd_) != 0) {
        std::cerr << "[ArbotixDriver] ERROR: Serial flush failed: " << std::strerror(errno) << std::endl;
        return false;
    }

    return true;
}

bool ArbotixDriver::is_valid_status_packet(const std::vector<uint8_t>& packet) const {
    /*
     * Minimum Dynamixel Protocol 1.0 status packet:
     *
     * FF FF ID LENGTH ERROR CHECKSUM
     *
     * Minimum size = 6 bytes
     */
    if (packet.size() < 6) {
        return false;
    }

    // Header.
    if (packet[0] != 0xFF || packet[1] != 0xFF) {
        return false;
    }

    // Check packet size. Total size = LENGTH + 4.
    const uint8_t length = packet[3];

    const std::size_t expected_size = static_cast<std::size_t>(length) + 4;

    if (packet.size() != expected_size) {
        return false;
    }

    /*
     * Dynamixel Protocol 1.0 checksum validation.
     *
     * ID + LENGTH + ERROR + PARAMETERS + CHECKSUM
     * must equal 0xFF modulo 256.
     */
    uint8_t sum = 0;

    for (std::size_t i = 2; i < packet.size(); ++i) {
        sum = static_cast<uint8_t>(sum + packet[i]);
    }

    if (sum != 0xFF) {
        return false;
    }

    return true;
}


std::vector<uint8_t> ArbotixDriver::read_bytes(std::size_t max_size) {
    std::vector<uint8_t> buffer;

    if (!is_open()) {
        std::cerr << "[ArbotixDriver] ERROR: Cannot read: serial port is not open" << std::endl;
        return buffer;
    }

    buffer.reserve(max_size);

    while (buffer.size() < max_size) {
        uint8_t byte = 0;

        const int n = ::read(serial_fd_, &byte, 1);

        if (n < 0) {
            std::cerr << "[ArbotixDriver] ERROR: Serial read failed: " << std::strerror(errno) << std::endl;
            break;
        }

        if (n == 0) {
            // Timeout
            break;
        }

        buffer.push_back(byte);

        // Remove bytes until FF FF is found.
        while (buffer.size() >= 2 && (buffer[0] != 0xFF || buffer[1] != 0xFF)) {
            buffer.erase(buffer.begin());
        }

        // Once ID and LENGTH are available, we know how many bytes constitute the complete packet.
        if (buffer.size() >= 4) {
            size_t expected_size = static_cast<std::size_t>(buffer[3]) + 4;
            if (buffer.size() >= expected_size) {
                break;
            }
        }
    }

    if (buffer.empty()) {
        std::cout << "[ArbotixDriver] Read timeout" << std::endl;
    }

    return buffer;
}


bool ArbotixDriver::read_status_packet(uint8_t expected_id, std::vector<uint8_t>& parameters, int timeout_ms) {
    parameters.clear();

    std::vector<uint8_t> packet;

    const auto start = std::chrono::steady_clock::now();

    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

        if (elapsed >= timeout_ms) {
            std::cerr << "[ArbotixDriver] WARNING: Timeout while waiting "
                << "for servo ID " << static_cast<int>(expected_id) << std::endl;
            return false;
        }

        /*
         * A position response contains 8 bytes:
         *
         * FF FF ID LENGTH ERROR POS_L POS_H CHECKSUM
         *
         * Generic register reads may have another size,
         * therefore read a reasonably sized packet.
         */
        std::vector<uint8_t> bytes = read_bytes(32);

        if (bytes.empty()) {
            // read() timed out.
            continue;
        }
        packet.insert(packet.end(), bytes.begin(), bytes.end());

        /*
         * Search for:
         *
         * FF FF ID LENGTH ERROR ... CHECKSUM
         */
        while (packet.size() >= 4) {
            // Synchronize to packet header.
            if (packet[0] != 0xFF || packet[1] != 0xFF) {
                packet.erase(packet.begin());
                continue;
            }

            const uint8_t id = packet[2];
            const uint8_t length = packet[3];

            /*
             * Dynamixel Protocol 1.0:
             *
             * total packet size =
             *
             * 2 header bytes
             * + ID
             * + LENGTH
             * + LENGTH bytes
             *
             * = LENGTH + 4
             */
            const size_t total_size = static_cast<std::size_t>(length) + 4;

            if (packet.size() < total_size) {
                // Need more bytes.
                break;
            }

            // Ignore packets from another servo.
            if (id != expected_id) {
                packet.erase(packet.begin(), packet.begin() + total_size);
                continue;
            }

            std::vector<uint8_t> complete_packet(packet.begin(), packet.begin() + total_size);

            if (!is_valid_status_packet(complete_packet)) {
                std::cerr << "[ArbotixDriver] Invalid status packet from servo" << std::endl;
                return false;
            }


            uint8_t error = complete_packet[4];

            parameters.clear();

            // Extract response parameters.
            for (size_t i = 5; i < complete_packet.size() - 1; ++i) {
                parameters.push_back(complete_packet[i]);
            }

            if (error != 0) {
                std::cerr << "[ArbotixDriver] WARNING: Servo ID "
                    << static_cast<int>(id) << " returned error byte 0x"
                    << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(error) << std::dec << std::endl;
                return false;
            }

            return true;
        }
    }
}


uint8_t ArbotixDriver::checksum(std::vector<uint8_t>& data) {
    uint32_t total = 0;

    for (size_t i = 0; i < data.size(); ++i) {
        total += data[i];
    }
    return static_cast<uint8_t>(255 - (total % 256));
}


bool ArbotixDriver::read_register(uint8_t servo_id, uint8_t address, uint8_t length, std::vector<uint8_t>& data) {
    /*
     * Dynamixel Protocol 1.0
     *
     * READ_DATA:
     *
     * FF FF
     * ID
     * LENGTH
     * 02
     * ADDRESS
     * READ_LENGTH
     * CHECKSUM
     */

    const uint8_t packet_length = 4;
    const uint8_t instruction = 0x02;

    std::vector<uint8_t> body{servo_id, packet_length, instruction, address, length};
    std::vector<uint8_t> packet{0xFF, 0xFF};

    packet.insert(packet.end(), body.begin(), body.end());
    packet.emplace_back(checksum(body));

    // Equivalent to: ser.reset_input_buffer()
    if (tcflush(serial_fd_, TCIFLUSH) != 0) {
        std::cerr << "[ArbotixDriver] WARNING: Could not flush RX buffer: "
            << std::strerror(errno) << std::endl;
    }

    if (!write_bytes(packet)) {
        return false;
    }
    // Global timeout while debugging serial communication.
    bool answer = read_status_packet(servo_id, data, 50);

    return answer;
}

}  // namespace phantomx_pincher_hardware