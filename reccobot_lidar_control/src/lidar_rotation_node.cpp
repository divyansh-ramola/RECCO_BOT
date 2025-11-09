#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"

using namespace std::chrono_literals;

class LidarRotationNode : public rclcpp::Node
{
public:
  LidarRotationNode()
  : Node("lidar_rotation_node"), current_position_(0.0), direction_(1.0)
  {
    // Publisher to the lidar controller command topic
    publisher_ = this->create_publisher<std_msgs::msg::Float64MultiArray>(
      "/lidar_controller/commands", 10);

    // Timer to update position at 50 Hz
    timer_ = this->create_wall_timer(
      20ms, std::bind(&LidarRotationNode::timer_callback, this));

    // Parameters
    this->declare_parameter("rotation_speed", 0.5);  // radians per second
    this->declare_parameter("min_angle", -6.28);     // radians (~-360 degrees)
    this->declare_parameter("max_angle", 6.25);      // radians (~360 degrees)

    rotation_speed_ = this->get_parameter("rotation_speed").as_double();
    min_angle_ = this->get_parameter("min_angle").as_double();
    max_angle_ = this->get_parameter("max_angle").as_double();

    RCLCPP_INFO(this->get_logger(), "Lidar Rotation Node started");
    RCLCPP_INFO(this->get_logger(), "Rotation speed: %.2f rad/s", rotation_speed_);
    RCLCPP_INFO(this->get_logger(), "Angle range: [%.2f, %.2f] rad", min_angle_, max_angle_);
  }

private:
  void timer_callback()
  {
    // Update position based on direction and speed
    double dt = 0.02;  // 20ms = 0.02 seconds
    current_position_ += direction_ * rotation_speed_ * dt;

    // Reverse direction if we hit the limits
    if (current_position_ >= max_angle_) {
      current_position_ = max_angle_;
      direction_ = -1.0;
      RCLCPP_INFO(this->get_logger(), "Reversing direction at max angle");
    } else if (current_position_ <= min_angle_) {
      current_position_ = min_angle_;
      direction_ = 1.0;
      RCLCPP_INFO(this->get_logger(), "Reversing direction at min angle");
    }

    // Publish command
    auto message = std_msgs::msg::Float64MultiArray();
    message.data = {current_position_};
    publisher_->publish(message);
  }

  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  double current_position_;
  double direction_;
  double rotation_speed_;
  double min_angle_;
  double max_angle_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<LidarRotationNode>());
  rclcpp::shutdown();
  return 0;
}
