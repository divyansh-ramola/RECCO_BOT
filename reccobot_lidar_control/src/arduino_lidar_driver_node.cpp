// Simple Arduino-driven lidar rotation driver.
// - Connects to Arduino over TCP
// - Reads lines like "angle:<rad>" from state_port and publishes JointState

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include <asio.hpp>
#include <string>
#include <memory>

using asio::ip::tcp;

class ArduinoLidarDriver : public rclcpp::Node
{
public:
  ArduinoLidarDriver()
  : Node("arduino_lidar_driver"), io_(), state_socket_(io_), cmd_socket_(io_)
  {
    this->declare_parameter<std::string>("ip", "192.168.1.150");
    this->declare_parameter<int>("state_port", 8000);
    this->declare_parameter<int>("command_port", 8001);
    this->declare_parameter<std::string>("joint_name", "base_coupling");

    this->get_parameter("ip", ip_);
    this->get_parameter("state_port", state_port_);
    this->get_parameter("command_port", command_port_);
    this->get_parameter("joint_name", joint_name_);

    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);

    connect();

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(5),
      std::bind(&ArduinoLidarDriver::poll, this));
  }

private:
  void connect()
  {
    try {
      tcp::resolver resolver(io_);
      auto state_endpoints = resolver.resolve(ip_, std::to_string(state_port_));
      asio::connect(state_socket_, state_endpoints);

      auto cmd_endpoints = resolver.resolve(ip_, std::to_string(command_port_));
      asio::connect(cmd_socket_, cmd_endpoints);

      RCLCPP_INFO(this->get_logger(), "Connected to Arduino at %s", ip_.c_str());
    } catch (const std::exception &e) {
      RCLCPP_ERROR(this->get_logger(), "Failed to connect to Arduino: %s", e.what());
    }
  }

  void poll()
  {
    if (!state_socket_.is_open()) {
      return;
    }

    asio::error_code ec;
    for (;;) {
      char c;
      size_t n = state_socket_.read_some(asio::buffer(&c, 1), ec);
      if (ec == asio::error::would_block || ec == asio::error::try_again) {
        break;
      }
      if (ec) {
        RCLCPP_WARN(this->get_logger(), "State socket error: %s", ec.message().c_str());
        state_socket_.close();
        break;
      }
      if (n == 1) {
        if (c == '\n') {
          handle_line(buffer_);
          buffer_.clear();
        } else if (c != '\r') {
          buffer_.push_back(c);
        }
      } else {
        break;
      }
    }
  }

  void handle_line(const std::string &line)
  {
    if (line.rfind("angle:", 0) == 0) {
      try {
        double angle = std::stod(line.substr(6));
        auto msg = sensor_msgs::msg::JointState();
        msg.header.stamp = this->now();
        msg.name.push_back(joint_name_);
        msg.position.push_back(angle);
        joint_pub_->publish(msg);
      } catch (const std::exception &e) {
        RCLCPP_WARN(this->get_logger(), "Failed to parse angle line '%s': %s", line.c_str(), e.what());
      }
    }
  }

  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  asio::io_context io_;
  tcp::socket state_socket_;
  tcp::socket cmd_socket_;
  std::string ip_;
  int state_port_;
  int command_port_;
  std::string joint_name_;
  std::string buffer_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ArduinoLidarDriver>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
