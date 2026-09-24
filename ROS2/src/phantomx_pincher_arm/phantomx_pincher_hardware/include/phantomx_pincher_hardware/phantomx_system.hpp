#ifndef PHANTOMX_PINCHER_HARDWARE__PHANTOMX_SYSTEM_HPP_
#define PHANTOMX_PINCHER_HARDWARE__PHANTOMX_SYSTEM_HPP_

#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/macros.hpp"
#include "rclcpp_lifecycle/state.hpp"

namespace phantomx_pincher_hardware {

class PhantomXSystem : public hardware_interface::SystemInterface {
public:
    RCLCPP_SHARED_PTR_DEFINITIONS(PhantomXSystem)

    hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo& info) override;
    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;
    hardware_interface::return_type read(const rclcpp::Time& time, const rclcpp::Duration& period) override;
    hardware_interface::return_type write(const rclcpp::Time& time, const rclcpp::Duration& period) override;

private:
    std::vector<double> hw_positions_;
    std::vector<double> hw_commands_;
};

}  // namespace phantomx_pincher_hardware

#endif  // PHANTOMX_PINCHER_HARDWARE__PHANTOMX_SYSTEM_HPP_