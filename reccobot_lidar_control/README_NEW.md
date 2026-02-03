# RECCOBOT LiDAR Control Package

Independent LiDAR control system for RECCOBOT with Arduino stepper motor control, ros2_control hardware interface, and custom position controller.

## 🚀 Quick Start

See [Quick Start Guide](docs/QUICK_START.md) for step-by-step instructions.

## 📚 Documentation

- **[Detailed Documentation](docs/LIDAR_CONTROL_README.md)** - Complete system documentation
- **[Quick Start Guide](docs/QUICK_START.md)** - Get up and running in minutes

## 📦 Package Contents

### Hardware Interface
- **Arduino Firmware**: Stepper motor control with serial communication
- **Hardware Plugin**: ros2_control integration for seamless ROS 2 integration

### Controller
- **LiDAR Position Controller**: Custom ros2_control controller with:
  - Manual positioning via ROS 2 topics
  - Automatic continuous scanning mode
  - Configurable scan speed and range
  - Real-time state feedback

### Examples & Tools
- **Python Control Example**: Demonstrates various control patterns
- **Launch Files**: Pre-configured launch files for easy deployment
- **Configuration Files**: YAML configs for controller parameters

## 🔧 Key Features

✅ **Plug-and-play** Arduino stepper motor control  
✅ **ros2_control** integration for standard ROS 2 workflows  
✅ **Automatic scanning** with configurable patterns  
✅ **Manual positioning** for targeted sensing  
✅ **Real-time feedback** at 100 Hz update rate  
✅ **High precision** - 6400 steps/revolution (0.056°/step)  

## 📋 Requirements

- ROS 2 (Humble/Iron/Rolling)
- Arduino (with TimerOne library)
- Stepper motor + driver
- ros2_control packages

## 🛠️ Installation

```bash
cd ~/reccobot_ws
colcon build --packages-select reccobot_lidar_control
source install/setup.bash
```

## 🎮 Usage Examples

### Launch Complete System
```bash
ros2 launch reccobot_lidar_control lidar_control.launch.py
```

### Manual Control
```bash
# Move to 90 degrees
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 1.5708" --once
```

### Run Example Node
```bash
ros2 run reccobot_lidar_control lidar_control_example.py
```

## 📁 Directory Structure

```
reccobot_lidar_control/
├── arduino/              # Arduino firmware
│   └── stepper_lidar_control/
├── config/               # Controller configurations
├── docs/                 # Comprehensive documentation
├── include/              # C++ headers
├── launch/               # Launch files
├── scripts/              # Python examples
└── src/                  # C++ implementations
```

## 🔌 Hardware Setup

### Wiring
```
Arduino Pin 2 (PUL) -> Stepper Driver PULSE/STEP
Arduino Pin 3 (DIR) -> Stepper Driver DIRECTION  
Arduino Pin 4 (ENA) -> Stepper Driver ENABLE
Arduino GND -> Driver GND
```

### Stepper Motor Configuration
- Steps per revolution: 6400 (configurable in Arduino code)
- Serial baud rate: 115200
- Command format: `cmd:<angle_in_radians>\n`
- State format: `angle:<current_angle_in_radians>\n`

## 🤝 Contributing

Contributions are welcome! Please check the documentation for development guidelines.

## 📄 License

[Your License]

## 🆘 Support

For issues and questions, see the [detailed documentation](docs/LIDAR_CONTROL_README.md) or open an issue.

---

**Note**: This package includes a complete independent LiDAR control system with:
- Arduino stepper motor firmware
- ros2_control hardware interface
- Custom position controller with scanning modes
- Example code and comprehensive documentation
