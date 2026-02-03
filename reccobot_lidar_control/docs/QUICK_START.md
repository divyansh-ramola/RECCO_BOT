# Quick Start Guide - LiDAR Control System

## Prerequisites

1. **Hardware**:
   - Arduino board (Uno/Nano/Mega)
   - Stepper motor + driver
   - LiDAR sensor
   - USB cable for Arduino

2. **Software**:
   - ROS 2 (Humble or later)
   - Arduino IDE or arduino-cli
   - TimerOne library for Arduino

## Installation Steps

### 1. Install Arduino Dependencies

**Option A: Using Arduino IDE**
```bash
# Install Arduino IDE if not already installed
sudo apt install arduino

# Install TimerOne library:
# Open Arduino IDE -> Tools -> Manage Libraries
# Search for "TimerOne" and install
```

**Option B: Using arduino-cli**
```bash
# Install arduino-cli
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Install TimerOne library
arduino-cli lib install TimerOne
```

### 2. Build the ROS 2 Package

```bash
cd ~/reccobot_ws
colcon build --packages-select reccobot_lidar_control
source install/setup.bash
```

### 3. Upload Arduino Firmware

**Wiring**:
```
Arduino Pin 2 (PUL) -> Driver PULSE/STEP
Arduino Pin 3 (DIR) -> Driver DIRECTION
Arduino Pin 4 (ENA) -> Driver ENABLE
Arduino GND -> Driver GND
Driver Power: Connect appropriate voltage for your motor
Motor: Connect to driver output
```

**Upload**:
```bash
# Using Arduino IDE:
# 1. Open: install/reccobot_lidar_control/share/reccobot_lidar_control/arduino/stepper_lidar_control/stepper_lidar_control.ino
# 2. Select Board: Tools -> Board -> Arduino Uno (or your board)
# 3. Select Port: Tools -> Port -> /dev/ttyUSB0 (or your port)
# 4. Click Upload button

# OR using arduino-cli:
cd ~/reccobot_ws/src/reccobot_lidar_control/arduino/stepper_lidar_control
arduino-cli compile --fqbn arduino:avr:uno .
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno .
```

### 4. Test Arduino Communication

```bash
# Install screen if needed
sudo apt install screen

# Open serial monitor
screen /dev/ttyUSB0 115200

# You should see:
# "Stepper + Serial ready"
# angle:0.000000
# angle:0.000000
# ...

# Type command:
cmd:1.5708

# Should respond:
# OK rad=1.570800

# Exit: Press Ctrl+A, then K, then Y
```

## Quick Test

### Test 1: Hardware Interface Only

```bash
# Terminal 1: Start controller manager with hardware
ros2 launch reccobot_lidar_control lidar_control.launch.py

# Terminal 2: Check hardware interfaces
ros2 control list_hardware_interfaces

# Should show:
# command_interfaces:
#   lidar_rotation_joint/position [available] [claimed]
# state_interfaces:
#   lidar_rotation_joint/position
```

### Test 2: Controller with Continuous Scan

```bash
# Start the system (continuous scan enabled by default)
ros2 launch reccobot_lidar_control lidar_control.launch.py

# Monitor the angle (should sweep back and forth)
ros2 topic echo /lidar_position_controller/state
```

### Test 3: Manual Control

```bash
# Terminal 1: Start system with scan disabled
ros2 launch reccobot_lidar_control lidar_control.launch.py

# Terminal 2: Send position commands
# Move to 0° (0 rad)
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 0.0" --once

# Wait 2 seconds, then move to 90° (π/2 rad)
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 1.5708" --once

# Wait 2 seconds, then move to 180° (π rad)
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 3.14159" --once

# Terminal 3: Monitor position
ros2 topic echo /lidar_position_controller/state
```

## Common Issues

### Issue: Arduino not detected

```bash
# Check if device exists
ls -l /dev/ttyUSB* /dev/ttyACM*

# Add user to dialout group
sudo usermod -a -G dialout $USER
# Logout and login again
```

### Issue: Motor not moving

1. Check enable pin (should be LOW to enable driver)
2. Verify driver power supply is on
3. Check current limit on driver
4. Test with manual Arduino commands via screen

### Issue: Controller not found

```bash
# Check if plugin is available
ros2 control list_controller_types | grep -i lidar

# Should show:
# reccobot_lidar_control/LidarPositionController

# If not, rebuild:
cd ~/reccobot_ws
colcon build --packages-select reccobot_lidar_control --cmake-clean-cache
source install/setup.bash
```

## Configuration Customization

Edit `config/lidar_controller.yaml`:

```yaml
lidar_position_controller:
  ros__parameters:
    joint: "lidar_rotation_joint"
    
    # Disable continuous scan for manual control only
    continuous_scan: false
    
    # Or adjust scan parameters
    scan_speed: 0.5              # Slower scan (rad/s)
    scan_min_angle: 0.0          # Start at 0°
    scan_max_angle: 3.14159      # Scan only 0° to 180°
```

Then restart:
```bash
ros2 launch reccobot_lidar_control lidar_control.launch.py controller_config_file:=lidar_controller.yaml
```

## Next Steps

1. **Integrate with your robot**: Update your main robot URDF to include the LiDAR joint
2. **Add LiDAR sensor**: Connect LiDAR sensor and integrate scan data
3. **Create mapping**: Use scan data with rotation angle for 3D mapping
4. **Optimize performance**: Tune scan speed and patterns for your application

## Getting Help

- Check detailed documentation: `docs/LIDAR_CONTROL_README.md`
- ROS 2 Control docs: https://control.ros.org/
- Package issues: [Your issue tracker]

## Example: Complete System Test

```bash
# 1. Upload Arduino firmware (one time)
cd ~/reccobot_ws/src/reccobot_lidar_control/arduino/stepper_lidar_control
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno .

# 2. Start ROS 2 control system
cd ~/reccobot_ws
source install/setup.bash
ros2 launch reccobot_lidar_control lidar_control.launch.py

# 3. In another terminal, test commands
source ~/reccobot_ws/install/setup.bash

# Send position commands
for angle in 0.0 0.785 1.571 2.356 3.142 3.927 4.712 5.498; do
    echo "Moving to $angle radians"
    ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: $angle" --once
    sleep 2
done

# 4. Monitor state in another terminal
ros2 topic echo /lidar_position_controller/state
```

Success! Your LiDAR control system is ready. 🎉
