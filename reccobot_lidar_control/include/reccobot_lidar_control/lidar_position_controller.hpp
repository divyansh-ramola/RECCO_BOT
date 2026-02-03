#ifndef RECCOBOT_LIDAR_CONTROL__LIDAR_POSITION_CONTROLLER_HPP_
#define RECCOBOT_LIDAR_CONTROL__LIDAR_POSITION_CONTROLLER_HPP_

#include <memory>
#include <string>
#include <vector>

#include "controller_interface/controller_interface.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"
#include "realtime_tools/realtime_buffer.hpp"
#include "std_msgs/msg/float64.hpp"

namespace reccobot_lidar_control
{

class LidarPositionController : public controller_interface::ControllerInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(LidarPositionController)

  controller_interface::InterfaceConfiguration command_interface_configuration() const override;

  controller_interface::InterfaceConfiguration state_interface_configuration() const override;

  controller_interface::return_type update(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  controller_interface::CallbackReturn on_init() override;

  controller_interface::CallbackReturn on_configure(
    const rclcpp_lifecycle::State & previous_state) override;

  controller_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;

  controller_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

protected:
  std::string joint_name_;
  
  // Command subscriber
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr command_subscriber_;
  realtime_tools::RealtimeBuffer<std::shared_ptr<std_msgs::msg::Float64>> rt_command_ptr_;
  
  // State publisher
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr state_publisher_;
  std::shared_ptr<std_msgs::msg::Float64> state_msg_;

  // Control parameters
  double command_timeout_;
  rclcpp::Time last_command_time_;
  
  // Scanning parameters
  bool continuous_scan_;
  double scan_speed_;  // rad/s
  double scan_min_angle_;
  double scan_max_angle_;
  double current_scan_target_;
  int scan_direction_;  // 1 for forward, -1 for backward

  void command_callback(const std::shared_ptr<std_msgs::msg::Float64> msg);
  double normalize_angle(double angle);
};

}  // namespace reccobot_lidar_control

#endif  // RECCOBOT_LIDAR_CONTROL__LIDAR_POSITION_CONTROLLER_HPP_
