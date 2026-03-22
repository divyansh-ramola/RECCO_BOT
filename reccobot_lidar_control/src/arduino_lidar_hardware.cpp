#include "reccobot_lidar_control/arduino_lidar_hardware.hpp"

#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>
#include <string>

#include <sys/socket.h>
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

  // Read parameters from URDF
  serial_port_ = info_.hardware_parameters.count("serial_port") > 0
                 ? info_.hardware_parameters.at("serial_port")
                 : "/dev/ttyUSB0";

  baud_rate_ = info_.hardware_parameters.count("baud_rate") > 0
               ? std::stoi(info_.hardware_parameters.at("baud_rate"))
               : 115200;

  // Initialize state
  hw_position_ = 0.0;
  hw_velocity_ = 0.0;
  hw_effort_ = 0.0;
  prev_hw_position_ = 0.0;
  hw_command_position_ = 0.0;

  connected_ = false;
  serial_fd_ = -1;
  rx_len_ = 0;

  // Validate we have exactly one joint
  if (info_.joints.size() != 1)
  {
    RCLCPP_FATAL(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Expected exactly 1 joint, got %zu", info_.joints.size());
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Validate joint has position command interface
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

  RCLCPP_INFO(
    rclcpp::get_logger("ArduinoLidarHardware"),
    "Initialized for joint '%s' (serial_port=%s, baud=%d)",
    joint.name.c_str(), serial_port_.c_str(), baud_rate_);

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
        joint.name, hardware_interface::HW_IF_POSITION, &hw_position_);
    }
    else if (state_if.name == hardware_interface::HW_IF_VELOCITY)
    {
      state_interfaces.emplace_back(
        joint.name, hardware_interface::HW_IF_VELOCITY, &hw_velocity_);
    }
    else if (state_if.name == hardware_interface::HW_IF_EFFORT)
    {
      state_interfaces.emplace_back(
        joint.name, hardware_interface::HW_IF_EFFORT, &hw_effort_);
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
    joint.name, hardware_interface::HW_IF_POSITION, &hw_command_position_);
  return command_interfaces;
}

hardware_interface::CallbackReturn ArduinoLidarHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("ArduinoLidarHardware"), "Activating...");

  if (!connect_to_arduino())
  {
    RCLCPP_ERROR(rclcpp::get_logger("ArduinoLidarHardware"), "Failed to connect to ESP32");
    return hardware_interface::CallbackReturn::ERROR;
  }

  // Try to read initial position
  double initial_angle = 0.0;
  // Give ESP a moment, then try a few reads
  usleep(100000);  // 100 ms
  for (int i = 0; i < 10; ++i)
  {
    if (read_angle_from_arduino(initial_angle))
    {
      break;
    }
    usleep(30000);  // 30 ms
  }

  hw_position_ = initial_angle;
  prev_hw_position_ = initial_angle;
  hw_command_position_ = initial_angle;
  hw_velocity_ = 0.0;
  hw_effort_ = 0.0;

  RCLCPP_INFO(
    rclcpp::get_logger("ArduinoLidarHardware"),
    "Activated (initial position: %.4f rad)", hw_position_);

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn ArduinoLidarHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  RCLCPP_INFO(rclcpp::get_logger("ArduinoLidarHardware"), "Deactivating...");
  disconnect_from_arduino();
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type ArduinoLidarHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & period)
{
  if (!connected_)
  {
    return hardware_interface::return_type::ERROR;
  }

  double angle = hw_position_;
  if (read_angle_from_arduino(angle))
  {
    double dt = period.seconds();
    if (dt > 0.0)
    {
      hw_velocity_ = (angle - prev_hw_position_) / dt;
    }
    prev_hw_position_ = hw_position_;
    hw_position_ = angle;
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

  if (!send_command_to_arduino(hw_command_position_))
  {
    RCLCPP_WARN_THROTTLE(
      rclcpp::get_logger("ArduinoLidarHardware"),
      *rclcpp::Clock::make_shared(), 1000,
      "Failed to send command to ESP32");
  }

  return hardware_interface::return_type::OK;
}

// ─────────────────── serial helpers ───────────────────

bool ArduinoLidarHardware::connect_to_arduino()
{
  RCLCPP_INFO(
    rclcpp::get_logger("ArduinoLidarHardware"),
    "Opening serial %s @ %d baud", serial_port_.c_str(), baud_rate_);

  serial_fd_ = open(serial_port_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
  if (serial_fd_ < 0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Failed to open %s: %s", serial_port_.c_str(), strerror(errno));
    return false;
  }

  struct termios tty;
  if (tcgetattr(serial_fd_, &tty) != 0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "tcgetattr failed: %s", strerror(errno));
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
  else if (baud_rate_ == 230400) speed = B230400;
  else if (baud_rate_ == 460800) speed = B460800;

  cfsetispeed(&tty, speed);
  cfsetospeed(&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;   // 8-bit chars
  tty.c_cflag |= (CLOCAL | CREAD);               // ignore modem, enable rx
  tty.c_cflag &= ~(PARENB | PARODD);             // no parity
  tty.c_cflag &= ~CSTOPB;                        // 1 stop bit
  tty.c_cflag &= ~CRTSCTS;                       // no hw flow control

  // Non-blocking: return immediately with whatever is available
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 1;   // 100 ms timeout

  if (tcsetattr(serial_fd_, TCSANOW, &tty) != 0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "tcsetattr failed: %s", strerror(errno));
    close(serial_fd_);
    serial_fd_ = -1;
    return false;
  }

  // Flush any stale data
  tcflush(serial_fd_, TCIOFLUSH);

  // Set non-blocking for the file descriptor
  int flags = fcntl(serial_fd_, F_GETFL, 0);
  fcntl(serial_fd_, F_SETFL, flags | O_NONBLOCK);

  rx_len_ = 0;
  connected_ = true;

  RCLCPP_INFO(
    rclcpp::get_logger("ArduinoLidarHardware"),
    "Connected to ESP32 on %s", serial_port_.c_str());
  return true;
}

void ArduinoLidarHardware::disconnect_from_arduino()
{
  if (serial_fd_ >= 0)
  {
    close(serial_fd_);
    serial_fd_ = -1;
  }
  connected_ = false;
  RCLCPP_INFO(rclcpp::get_logger("ArduinoLidarHardware"), "Disconnected from ESP32");
}

bool ArduinoLidarHardware::read_angle_from_arduino(double & angle)
{
  if (serial_fd_ < 0) return false;

  // Read whatever bytes are available from the UART into our ring buffer
  while (true)
  {
    ssize_t n = ::read(serial_fd_, rx_buf_ + rx_len_, sizeof(rx_buf_) - rx_len_);
    if (n > 0)
    {
      rx_len_ += static_cast<size_t>(n);
    }
    else
    {
      break;  // EAGAIN / no more data
    }
  }

  constexpr size_t PKT_SIZE = sizeof(AnglePacket);  // 8 bytes

  // We may have accumulated several telemetry packets.
  // Consume all complete packets, keep only the latest value.
  bool got_any = false;
  while (rx_len_ >= PKT_SIZE)
  {
    AnglePacket pkt;
    std::memcpy(&pkt, rx_buf_, PKT_SIZE);

    angle = pkt.angle_urad / 1000000.0;
    got_any = true;

    // Shift remaining bytes forward
    rx_len_ -= PKT_SIZE;
    if (rx_len_ > 0)
    {
      std::memmove(rx_buf_, rx_buf_ + PKT_SIZE, rx_len_);
    }
  }

  // Safety: if the buffer is getting full without producing a valid packet
  // something is misaligned — flush it.
  if (rx_len_ > PKT_SIZE * 4)
  {
    RCLCPP_WARN(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "RX buffer overflow (%zu bytes), flushing", rx_len_);
    rx_len_ = 0;
    tcflush(serial_fd_, TCIFLUSH);
  }

  return got_any;
}

bool ArduinoLidarHardware::send_command_to_arduino(double angle)
{
  if (serial_fd_ < 0) return false;

  CmdPacket pkt;
  pkt.target_urad = static_cast<int32_t>(angle * 1000000.0);

  ssize_t n = ::write(serial_fd_, reinterpret_cast<const uint8_t *>(&pkt), sizeof(pkt));
  if (n < 0)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("ArduinoLidarHardware"),
      "Serial write failed: %s", strerror(errno));
    return false;
  }
  return (static_cast<size_t>(n) == sizeof(pkt));
}

}  // namespace reccobot_lidar_control

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  reccobot_lidar_control::ArduinoLidarHardware, hardware_interface::SystemInterface)
