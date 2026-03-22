#ifndef RECCOBOT_LIDAR_CONTROL__ARDUINO_LIDAR_HARDWARE_HPP_
#define RECCOBOT_LIDAR_CONTROL__ARDUINO_LIDAR_HARDWARE_HPP_

#include <cstdint>
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

// Binary packet structs matching the ESP32 firmware exactly
#pragma pack(push, 1)
struct AnglePacket   // ESP → RPI  (8 bytes)
{
  int32_t  angle_urad;   // current angle in micro-radians
  uint32_t t_us;         // ESP micros() timestamp
};

struct CmdPacket     // RPI → ESP  (4 bytes)
{
  int32_t  target_urad;  // target angle in micro-radians
};
#pragma pack(pop)

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
  // Serial connection parameters
  std::string serial_port_;
  int baud_rate_;
  int serial_fd_;

  // Joint state
  double hw_position_;
  double hw_velocity_;
  double hw_effort_;
  double prev_hw_position_;
  rclcpp::Time prev_read_time_;

  // Joint command
  double hw_command_position_;

  bool connected_;

  // Receive buffer for accumulating partial binary reads
  uint8_t rx_buf_[256];
  size_t rx_len_;

  // Helper methods
  bool connect_to_arduino();
  void disconnect_from_arduino();
  bool read_angle_from_arduino(double & angle);
  bool send_command_to_arduino(double angle);
};

}  // namespace reccobot_lidar_control

#endif  // RECCOBOT_LIDAR_CONTROL__ARDUINO_LIDAR_HARDWARE_HPP_
