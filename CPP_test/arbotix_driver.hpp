#ifndef PHANTOMX_PINCHER_HARDWARE__ARBOTIX_DRIVER_HPP_
#define PHANTOMX_PINCHER_HARDWARE__ARBOTIX_DRIVER_HPP_

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <termios.h>

namespace phantomx_pincher_hardware
{

class ArbotixDriver
{
public:
    ArbotixDriver();
    ~ArbotixDriver();

    bool open(const std::string & port, int baud_rate);
    void close();
    bool is_open() const;
    bool write_bytes(const std::vector<uint8_t> & data);
    std::vector<uint8_t> read_bytes(std::size_t max_size);
    bool read_register(uint8_t servo_id, uint8_t address, uint8_t length, std::vector<uint8_t> & data);

private:
    bool configure_port(int baud_rate);
    uint8_t checksum(std::vector<uint8_t>& data);
    bool is_valid_status_packet(const std::vector<uint8_t>& packet) const;
    bool read_status_packet(uint8_t expected_id, std::vector<uint8_t> & parameters, int timeout_ms);

    int serial_fd_;
    std::string port_;
    std::map<int, speed_t> speedsMap_;
};

}  // namespace phantomx_pincher_hardware

#endif