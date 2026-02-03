#include "reccobot_lidar_control/arduino_lidar_hardware.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

namespace reccobot_lidar_control
{

hardware_interface::CallbackReturn ArduinoLidarHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  if (
    hardware_interface::SystemInterface::on_init(info) !=
    hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Read parameters from URDF - support both serial and TCP modes
  if (info_.hardware_parameters.count("serial_port") > 0) {
    // Serial mode
    serial_port_ = info_.hardware_parameters["serial_port"];
    baud_rate_ = info_.hardware_parameters.count("baud_rate") > 0 
                 ? std::stoi(info_.hardware_parameters["baud_rate"]) 
                 : 115200;
    use_serial_ = true;
  } else {
    // TCP mode (legacy)
    ip_ = info_.hardware_parameters["ip"];
    state_port_ = std::stoi(info_.hardware_parameters["state_port"]);
    command_port_ = std::stoi(info_.hardware_parameters["command_port"]);
    use_serial_ = false;
  }

  // Initialize state
  hw_position_ = std::numeric_limits<double>::quiet_NaN();
  hw_velocity_ = std::numeric_limits<double>::quiet_NaN();
  hw_effort_ = std::numeric_limits<double>::quiet_NaN();
  hw_command_position_ = 0.0;

  connected_ = false;
  state_socket_fd_ = -1;
  command_socket_fd_ = -1;
  serial_fd_ = -1;

  // Validate we have exactly one joint
  if (info_.joints.size() != 1)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Expected exactly 1 joint, got %zu", info_.joints.size());
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Validate joint has position command and state interfaces
  const auto & joint = info_.joints[0];
  if (joint.command_interfaces.size() != 1 ||
      joint.command_interfaces[0].name != hardware_interface::HW_IF_POSITION)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Joint '%s' must have exactly 1 position command interface", joint.name.c_str());
    return hardware_interface::CallbackReturn::ERROR;
  }

  if (joint.state_interfaces.size() < 1)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Joint '%s' must have at least 1 state interface", joint.name.c_str());
    return hardware_interface::CallbackReturn::ERROR;
  }

  if (use_serial_) {
    RCLCPP_INFO(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Initialized Arduino Lidar Hardware for joint '%s' (serial_port=%s, baud=%d)",
      joint.name.c_str(), serial_port_.c_str(), baud_rate_);
  } else {
    RCLCPP_INFO(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Initialized Arduino Lidar Hardware for joint '%s' (ip=%s, state_port=%d, cmd_port=%d)",
      joint.name.c_str(), ip_.c_str(), state_port_, command_port_);
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface>
ArduinoLidarHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  const auto & joint = info_.joints[0];
  
  for (const auto & state_if : joint.state_interfaces)
  {
    if (state_if.name == hardware_interface::HW_IF_POSITION)
    {
      state_interfaces.emplace_back(
        hardware_interface::StateInterface(
          joint.name, hardware_interface::HW_IF_POSITION, &hw_position_));
    }
    else if (state_if.name == hardware_interface::HW_IF_VELOCITY)
    {
      state_interfaces.emplace_back(
        hardware_interface::StateInterface(
          joint.name, hardware_interface::HW_IF_VELOCITY, &hw_velocity_));
    }
    else if (state_if.name == hardware_interface::HW_IF_EFFORT)
    {
      state_interfaces.emplace_back(
        hardware_interface::StateInterface(
          joint.name, hardware_interface::HW_IF_EFFORT, &hw_effort_));
    }
  }

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface>
ArduinoLidarHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  const auto & joint = info_.joints[0];

  command_interfaces.emplace_back(
    hardware_interface::CommandInterface(
      joint.name, hardware_interface::HW_IF_POSITION, &hw_command_position_));

  return command_interfaces;
}

hardware_interface::CallbackReturn ArduinoLidarHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("ArduinoLidarHardware"), "Activating Arduino Lidar Hardware...");

  if (!connect_to_arduino())
  {
    RCLCPP_ERROR(rclcpp::get_logger("ArduinoLidarHardware"), "Failed to connect to Arduino");
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Initialize position from Arduino
  double initial_angle = 0.0;
  if (read_angle_from_arduino(initial_angle))
  {
    hw_position_ = initial_angle;
    hw_command_position_ = initial_angle;
  }
  else
  {
    hw_position_ = 0.0;
    hw_command_position_ = 0.0;
  }

  hw_velocity_ = 0.0;
  hw_effort_ = 0.0;

  RCLCPP_INFO(
    rclcpp::get_logger("ArduinoLidarHardware"),
    "Activated successfully (initial position: %.3f rad)", hw_position_);

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn ArduinoLidarHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("ArduinoLidarHardware"), "Deactivating Arduino Lidar Hardware...");
  
  disconnect_from_arduino();

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type ArduinoLidarHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  if (!connected_)
  {
    return hardware_interface::return_type::ERROR;
  }

  double angle = 0.0;
  if (read_angle_from_arduino(angle))
  {
    hw_position_ = angle;
    // Could compute velocity from position delta if needed
  }

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type ArduinoLidarHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  if (!connected_)
  {
    return hardware_interface::return_type::ERROR;
  }

  // Send command to Arduino
  if (!send_command_to_arduino(hw_command_position_))
  {
    RCLCPP_WARN_THROTTLE(
      rclcpp::get_logger("ArduinoLidarHardware"),
      *rclcpp::Clock::make_shared(), 1000,
      "Failed to send command to Arduino");
  }

  return hardware_interface::return_type::OK;
}

bool ArduinoLidarHardware::connect_to_arduino()
{
  if (use_serial_)
  {
    RCLCPP_INFO(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Using serial mode (port=%s, baud=%d)",
      serial_port_.c_str(), baud_rate_);

    if (serial_port_.empty())
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("ArduinoLidarHardware"),
        "Serial port is empty; cannot connect in serial mode");
      return false;
    }

    serial_fd_ = open(serial_port_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd_ < 0)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("ArduinoLidarHardware"),
        "Failed to open serial port %s: %s",
        serial_port_.c_str(), strerror(errno));
      return false;
    }

    struct termios tty;
    if (tcgetattr(serial_fd_, &tty) != 0)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("ArduinoLidarHardware"),
        "Failed to get serial attributes: %s", strerror(errno));
      close(serial_fd_);
      serial_fd_ = -1;
      return false;
    }

    cfmakeraw(&tty);

    speed_t speed = B115200;
    if (baud_rate_ == 9600) speed = B9600;
    else if (baud_rate_ == 19200) speed = B19200;
    else if (baud_rate_ == 38400) speed = B38400;
    else if (baud_rate_ == 57600) speed = B57600;
    else if (baud_rate_ == 115200) speed = B115200;

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(serial_fd_, TCSANOW, &tty) != 0)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("ArduinoLidarHardware"),
        "Failed to set serial attributes: %s", strerror(errno));
      close(serial_fd_);
      serial_fd_ = -1;
      return false;
    }

    int flags = fcntl(serial_fd_, F_GETFL, 0);
    fcntl(serial_fd_, F_SETFL, flags | O_NONBLOCK);

    connected_ = true;
    RCLCPP_INFO(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Connected to Arduino via serial %s (baud=%d)",
      serial_port_.c_str(), baud_rate_);
    return true;
  }

  // Create state socket
  state_socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (state_socket_fd_ < 0)
  {
    RCLCPP_ERROR(rclcpp::get_logger("ArduinoLidarHardware"), "Failed to create state socket");
    return false;
  }

  // Create command socket
  command_socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (command_socket_fd_ < 0)
  {
    RCLCPP_ERROR(rclcpp::get_logger("ArduinoLidarHardware"), "Failed to create command socket");
    close(state_socket_fd_);
    state_socket_fd_ = -1;
    return false;
  }

  // Connect state socket
  struct sockaddr_in state_addr;
  state_addr.sin_family = AF_INET;
  state_addr.sin_port = htons(state_port_);
  inet_pton(AF_INET, ip_.c_str(), &state_addr.sin_addr);

  if (connect(state_socket_fd_, (struct sockaddr*)&state_addr, sizeof(state_addr)) < 0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Failed to connect to Arduino state port %s:%d - %s",
      ip_.c_str(), state_port_, strerror(errno));
    close(state_socket_fd_);
    close(command_socket_fd_);
    state_socket_fd_ = -1;
    command_socket_fd_ = -1;
    return false;
  }

  // Connect command socket
  struct sockaddr_in command_addr;
  command_addr.sin_family = AF_INET;
  command_addr.sin_port = htons(command_port_);
  inet_pton(AF_INET, ip_.c_str(), &command_addr.sin_addr);

  if (connect(command_socket_fd_, (struct sockaddr*)&command_addr, sizeof(command_addr)) < 0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Failed to connect to Arduino command port %s:%d - %s",
      ip_.c_str(), command_port_, strerror(errno));
    close(state_socket_fd_);
    close(command_socket_fd_);
    state_socket_fd_ = -1;
    command_socket_fd_ = -1;
    return false;
  }

  // Set state socket to non-blocking
  int flags = fcntl(state_socket_fd_, F_GETFL, 0);
  fcntl(state_socket_fd_, F_SETFL, flags | O_NONBLOCK);

  connected_ = true;

  RCLCPP_INFO(
    rclcpp::get_logger("ArduinoLidarHardware"),
    "Connected to Arduino at %s (state:%d, cmd:%d)",
    ip_.c_str(), state_port_, command_port_);

  return true;
}

void ArduinoLidarHardware::disconnect_from_arduino()
{
  if (serial_fd_ >= 0)
  {
    close(serial_fd_);
    serial_fd_ = -1;
  }

  if (state_socket_fd_ >= 0)
  {
    close(state_socket_fd_);
    state_socket_fd_ = -1;
  }

  if (command_socket_fd_ >= 0)
  {
    close(command_socket_fd_);
    command_socket_fd_ = -1;
  }

  connected_ = false;

  RCLCPP_INFO(rclcpp::get_logger("ArduinoLidarHardware"), "Disconnected from Arduino");
}

bool ArduinoLidarHardware::read_angle_from_arduino(double & angle)
{
  if (use_serial_)
  {
    if (serial_fd_ < 0) return false;

    static std::string buffer;
    char temp[256];

    while (true)
    {
  ssize_t n = ::read(serial_fd_, temp, sizeof(temp) - 1);
      if (n > 0)
      {
        temp[n] = '\0';
        buffer += temp;
      }
      else
      {
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
          break;
        }
        if (n == 0)
        {
          break;
        }
        if (n < 0)
        {
          RCLCPP_ERROR(
            rclcpp::get_logger("ArduinoLidarHardware"),
            "Error reading from Arduino serial: %s", strerror(errno));
          return false;
        }
      }
    }

    size_t pos;
    while ((pos = buffer.find('\n')) != std::string::npos)
    {
      std::string line = buffer.substr(0, pos);
      buffer.erase(0, pos + 1);

      if (line.rfind("angle:", 0) == 0)
      {
        try
        {
          angle = std::stod(line.substr(6));
          return true;
        }
        catch (const std::exception & e)
        {
          RCLCPP_WARN(
            rclcpp::get_logger("ArduinoLidarHardware"),
            "Failed to parse angle from line '%s': %s", line.c_str(), e.what());
        }
      }
    }

    return false;
  }

  if (state_socket_fd_ < 0) return false;

  // Read available data (non-blocking)
  static std::string buffer;
  char temp[256];

  while (true)
  {
    ssize_t n = recv(state_socket_fd_, temp, sizeof(temp) - 1, 0);
    
    if (n > 0)
    {
      temp[n] = '\0';
      buffer += temp;
    }
    else if (n == 0)
    {
      // Connection closed
      RCLCPP_ERROR(rclcpp::get_logger("ArduinoLidarHardware"), "Arduino state connection closed");
      connected_ = false;
      return false;
    }
    else
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        // No more data available
        break;
      }
      else
      {
        // Error
        RCLCPP_ERROR(
          rclcpp::get_logger("ArduinoLidarHardware"),
          "Error reading from Arduino: %s", strerror(errno));
        return false;
      }
    }
  }

  // Parse buffer for "angle:<value>\n" lines
  size_t pos;
  while ((pos = buffer.find('\n')) != std::string::npos)
  {
    std::string line = buffer.substr(0, pos);
    buffer.erase(0, pos + 1);

    if (line.rfind("angle:", 0) == 0)
    {
      try
      {
        angle = std::stod(line.substr(6));
        return true;
      }
      catch (const std::exception & e)
      {
        RCLCPP_WARN(
          rclcpp::get_logger("ArduinoLidarHardware"),
          "Failed to parse angle from line '%s': %s", line.c_str(), e.what());
      }
    }
  }

  // No complete line yet
  return false;
}

bool ArduinoLidarHardware::send_command_to_arduino(double angle)
{
  if (use_serial_)
  {
    if (serial_fd_ < 0) return false;

    std::ostringstream oss;
    oss << "cmd:" << angle << "\n";
    std::string msg = oss.str();

  ssize_t n = ::write(serial_fd_, msg.c_str(), msg.size());
    if (n < 0)
    {
      RCLCPP_ERROR(
        rclcpp::get_logger("ArduinoLidarHardware"),
        "Failed to send command to Arduino serial: %s", strerror(errno));
      return false;
    }
    return true;
  }

  if (command_socket_fd_ < 0) return false;

  std::ostringstream oss;
  oss << "cmd:" << angle << "\n";
  std::string msg = oss.str();

  ssize_t n = send(command_socket_fd_, msg.c_str(), msg.size(), 0);

  if (n < 0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Failed to send command to Arduino: %s", strerror(errno));
    return false;
  }

  return true;
}

}  // namespace reccobot_lidar_control

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  reccobot_lidar_control::ArduinoLidarHardware, hardware_interface::SystemInterface)
