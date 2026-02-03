#ifndef RECCOBOT_LIDAR_CONTROL__ARDUINO_LIDAR_HARDWARE_HPP_
#define RECCOBOT_LIDAR_CONTROL__ARDUINO_LIDAR_HARDWARE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/macros.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"

namespace reccobot_lidar_control
{

class ArduinoLidarHardware : public hardware_interface::SystemInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(ArduinoLidarHardware)

  hardware_interface::CallbackReturn on_init(
    const hardware_interface::HardwareInfo & info) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::return_type read(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  hardware_interface::return_type write(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

private:
  // Arduino connection parameters - TCP mode
  std::string ip_;
  int state_port_;
  int command_port_;
  
  // Arduino connection parameters - Serial mode
  std::string serial_port_;
  int baud_rate_;
  bool use_serial_;
  int serial_fd_;

  // Joint state
  double hw_position_;
  double hw_velocity_;
  double hw_effort_;

  // Joint command
  double hw_command_position_;

  // TCP connection state (we'll use simple socket API or asio)
  int state_socket_fd_;
  int command_socket_fd_;
  bool connected_;

  // Helper methods
  bool connect_to_arduino();
  void disconnect_from_arduino();
  bool read_angle_from_arduino(double & angle);
  bool send_command_to_arduino(double angle);
};

}  // namespace reccobot_lidar_control

#endif  // RECCOBOT_LIDAR_CONTROL__ARDUINO_LIDAR_HARDWARE_HPP_
