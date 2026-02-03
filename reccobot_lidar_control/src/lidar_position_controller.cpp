#include "reccobot_lidar_control/lidar_position_controller.hpp"

#include <algorithm>
#include <memory>
#include <string>

#include "controller_interface/helpers.hpp"
#include "hardware_interface/loaned_command_interface.hpp"
#include "hardware_interface/loaned_state_interface.hpp"
#include "rclcpp/logging.hpp"
#include "rclcpp/qos.hpp"

namespace reccobot_lidar_control
{

controller_interface::CallbackReturn LidarPositionController::on_init()
{
  try
  {
    auto_declare<std::string>("joint", "");
    auto_declare<double>("command_timeout", 0.5);
    auto_declare<bool>("continuous_scan", false);
    auto_declare<double>("scan_speed", 1.0);
    auto_declare<double>("scan_min_angle", 0.0);
    auto_declare<double>("scan_max_angle", 6.28318530718);
  }
  catch (const std::exception & e)
  {
    RCLCPP_ERROR(
      get_node()->get_logger(),
      "Exception thrown during init stage with message: %s", e.what());
    return controller_interface::CallbackReturn::ERROR;
  }

  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn LidarPositionController::on_configure(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  joint_name_ = get_node()->get_parameter("joint").as_string();
  command_timeout_ = get_node()->get_parameter("command_timeout").as_double();
  continuous_scan_ = get_node()->get_parameter("continuous_scan").as_bool();
  scan_speed_ = get_node()->get_parameter("scan_speed").as_double();
  scan_min_angle_ = get_node()->get_parameter("scan_min_angle").as_double();
  scan_max_angle_ = get_node()->get_parameter("scan_max_angle").as_double();

  if (joint_name_.empty())
  {
    RCLCPP_ERROR(get_node()->get_logger(), "Joint parameter is empty!");
    return controller_interface::CallbackReturn::ERROR;
  }

  // Initialize scanning state
  current_scan_target_ = scan_min_angle_;
  scan_direction_ = 1;

  // Create command subscriber
  command_subscriber_ = get_node()->create_subscription<std_msgs::msg::Float64>(
    "~/command",
    rclcpp::SystemDefaultsQoS(),
    [this](const std::shared_ptr<std_msgs::msg::Float64> msg)
    {
      command_callback(msg);
    });

  // Create state publisher
  state_publisher_ = get_node()->create_publisher<std_msgs::msg::Float64>(
    "~/state", rclcpp::SystemDefaultsQoS());

  state_msg_ = std::make_shared<std_msgs::msg::Float64>();

  RCLCPP_INFO(
    get_node()->get_logger(),
    "Configured LidarPositionController for joint '%s' (continuous_scan=%s, scan_speed=%.3f rad/s)",
    joint_name_.c_str(),
    continuous_scan_ ? "true" : "false",
    scan_speed_);

  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::InterfaceConfiguration
LidarPositionController::command_interface_configuration() const
{
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  config.names.push_back(joint_name_ + "/position");
  return config;
}

controller_interface::InterfaceConfiguration
LidarPositionController::state_interface_configuration() const
{
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  config.names.push_back(joint_name_ + "/position");
  return config;
}

controller_interface::CallbackReturn LidarPositionController::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // Clear command buffer
  rt_command_ptr_.writeFromNonRT(std::shared_ptr<std_msgs::msg::Float64>());
  last_command_time_ = get_node()->now();

  RCLCPP_INFO(get_node()->get_logger(), "LidarPositionController activated");
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn LidarPositionController::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  // Release interfaces
  release_interfaces();

  RCLCPP_INFO(get_node()->get_logger(), "LidarPositionController deactivated");
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::return_type LidarPositionController::update(
  const rclcpp::Time & time, const rclcpp::Duration & period)
{
  // Read current state
  double current_position = state_interfaces_[0].get_value();

  // Publish current state
  state_msg_->data = current_position;
  state_publisher_->publish(*state_msg_);

  // Get command from realtime buffer
  auto command_msg = rt_command_ptr_.readFromRT();
  
  double target_position = 0.0;
  bool has_valid_command = false;

  if (command_msg && *command_msg)
  {
    // Check if command is not too old
    if ((time - last_command_time_).seconds() < command_timeout_)
    {
      target_position = normalize_angle((*command_msg)->data);
      has_valid_command = true;
    }
  }

  // If no valid command and continuous scan is enabled, generate scanning motion
  if (!has_valid_command && continuous_scan_)
  {
    // Update scan target based on scan speed
    double angle_increment = scan_speed_ * period.seconds();
    current_scan_target_ += scan_direction_ * angle_increment;

    // Check boundaries and reverse direction if needed
    if (current_scan_target_ >= scan_max_angle_)
    {
      current_scan_target_ = scan_max_angle_;
      scan_direction_ = -1;
    }
    else if (current_scan_target_ <= scan_min_angle_)
    {
      current_scan_target_ = scan_min_angle_;
      scan_direction_ = 1;
    }

    target_position = current_scan_target_;
  }
  else if (!has_valid_command)
  {
    // No command and no continuous scan - hold current position
    target_position = current_position;
  }

  // Write command to hardware interface
  command_interfaces_[0].set_value(target_position);

  return controller_interface::return_type::OK;
}

void LidarPositionController::command_callback(
  const std::shared_ptr<std_msgs::msg::Float64> msg)
{
  rt_command_ptr_.writeFromNonRT(msg);
  last_command_time_ = get_node()->now();
  RCLCPP_INFO_THROTTLE(
    get_node()->get_logger(),
    *get_node()->get_clock(),
    1000,
    "Received command: %.6f rad", msg->data);
}

double LidarPositionController::normalize_angle(double angle)
{
  const double TWO_PI = 6.28318530718;
  while (angle < 0.0) angle += TWO_PI;
  while (angle >= TWO_PI) angle -= TWO_PI;
  return angle;
}

}  // namespace reccobot_lidar_control

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(
  reccobot_lidar_control::LidarPositionController,
  controller_interface::ControllerInterface)
