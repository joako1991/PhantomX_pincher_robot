#include "phantomx_pincher_hardware/phantomx_system.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "pluginlib/class_list_macros.hpp"
#include "rclcpp/rclcpp.hpp"

namespace phantomx_pincher_hardware {

hardware_interface::CallbackReturn PhantomXSystem::on_init(const hardware_interface::HardwareInfo& info) {
    if (hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) {
        return hardware_interface::CallbackReturn::ERROR;
    }

    // Variables availables in the URDF file phantomx_pincher.urdf.xacro
    auto port_it = info_.hardware_parameters.find("port");
    auto baud_it = info_.hardware_parameters.find("baud_rate");

    if (port_it == info_.hardware_parameters.end() || baud_it == info_.hardware_parameters.end()) {
        RCLCPP_ERROR(rclcpp::get_logger("PhantomXSystem"), "Missing hardware parameters 'port' and/or 'baud_rate'");
        return hardware_interface::CallbackReturn::ERROR;
    }

    const std::string port = port_it->second;
    const int baud_rate = std::stoi(baud_it->second);

    if (!arbotix_driver_.open(port, baud_rate)) {
        RCLCPP_ERROR(rclcpp::get_logger("PhantomXSystem"), "Could not open ArbotiX serial port");
        return hardware_interface::CallbackReturn::ERROR;
    }

    hw_positions_.resize(info_.joints.size(), 0.0);
    hw_commands_.resize(info_.joints.size(), 0.0);

  RCLCPP_INFO(rclcpp::get_logger("PhantomXSystem"), "Initializing PhantomX mock hardware with %zu joints", info_.joints.size());

  /*
   * Verify that every joint exposes exactly:
   *
   *   command interface: position
   *   state interface:   position
   */
    for (const auto & joint : info_.joints) {
        if (joint.command_interfaces.size() != 1) {
            RCLCPP_ERROR(rclcpp::get_logger("PhantomXSystem"),
                "Joint '%s' has %zu command interfaces. Expected 1.",
                joint.name.c_str(),
                joint.command_interfaces.size());

            return hardware_interface::CallbackReturn::ERROR;
        }

        if (joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION) {
            RCLCPP_ERROR(rclcpp::get_logger("PhantomXSystem"),
                "Joint '%s' command interface is '%s'. Expected 'position'.",
                joint.name.c_str(),
                joint.command_interfaces[0].name.c_str());

            return hardware_interface::CallbackReturn::ERROR;
        }

        if (joint.state_interfaces.size() != 1) {
            RCLCPP_ERROR(rclcpp::get_logger("PhantomXSystem"),
                "Joint '%s' has %zu state interfaces. Expected 1.",
                joint.name.c_str(),
                joint.state_interfaces.size());

            return hardware_interface::CallbackReturn::ERROR;
        }

        if (joint.state_interfaces[0].name != hardware_interface::HW_IF_POSITION) {
            RCLCPP_ERROR(rclcpp::get_logger("PhantomXSystem"),
                "Joint '%s' state interface is '%s'. Expected 'position'.",
                joint.name.c_str(),
                joint.state_interfaces[0].name.c_str());

            return hardware_interface::CallbackReturn::ERROR;
        }
    }

    return hardware_interface::CallbackReturn::SUCCESS;
}


std::vector<hardware_interface::StateInterface> PhantomXSystem::export_state_interfaces() {
    std::vector<hardware_interface::StateInterface> state_interfaces;

    for (std::size_t i = 0; i < info_.joints.size(); ++i) {
        state_interfaces.emplace_back(
            hardware_interface::StateInterface(\
                info_.joints[i].name,
                hardware_interface::HW_IF_POSITION, &hw_positions_[i]
            )
        );
    }
    return state_interfaces;
}


std::vector<hardware_interface::CommandInterface> PhantomXSystem::export_command_interfaces() {
    std::vector<hardware_interface::CommandInterface> command_interfaces;

    for (std::size_t i = 0; i < info_.joints.size(); ++i) {
        command_interfaces.emplace_back(
            hardware_interface::CommandInterface(
                info_.joints[i].name,
                hardware_interface::HW_IF_POSITION, &hw_commands_[i]
            )
        );
    }

    return command_interfaces;
}


hardware_interface::return_type PhantomXSystem::read(const rclcpp::Time& time,  const rclcpp::Duration& period) {
    /*
     * MOCK HARDWARE:
     *
     * The simulated joint position is simply the last command
     * received by the hardware.
    */
    hw_positions_ = hw_commands_;

    return hardware_interface::return_type::OK;
}


hardware_interface::return_type PhantomXSystem::write(const rclcpp::Time& time, const rclcpp::Duration& period) {
    /*
     * MOCK HARDWARE:
     *
     * Nothing has to be transmitted yet.
     *
     * Later, this method will:
     *
     *   1. convert radians/metres to Dynamixel units
     *   2. build the ArbotiX command
     *   3. send it through the serial port
    */

    return hardware_interface::return_type::OK;
}
}  // namespace phantomx_pincher_hardware


PLUGINLIB_EXPORT_CLASS(phantomx_pincher_hardware::PhantomXSystem, hardware_interface::SystemInterface)