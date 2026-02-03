# RECCOBOT LiDAR Control Package

This package provides an independent LiDAR control system for the RECCOBOT, including Arduino stepper motor control, ros2_control hardware interface, and a custom controller for LiDAR positioning and scanning.

## Package Structure

```
reccobot_lidar_control/
├── arduino/
│   └── stepper_lidar_control/
│       └── stepper_lidar_control.ino     # Arduino firmware for stepper motor
├── config/
│   └── lidar_controller.yaml             # Controller configuration
├── include/reccobot_lidar_control/
│   ├── arduino_lidar_hardware.hpp        # Hardware interface header
│   └── lidar_position_controller.hpp     # Controller header
├── launch/
│   ├── lidar_control.launch.py           # Complete control system launch
│   └── lidar_controller_spawner.launch.py # Controller spawner only
├── src/
│   ├── arduino_lidar_hardware.cpp        # Hardware interface implementation
│   ├── lidar_position_controller.cpp     # Controller implementation
│   └── [other existing nodes...]
├── CMakeLists.txt
└── package.xml
```

## Components

### 1. Arduino Stepper Motor Control

**File**: `arduino/stepper_lidar_control/stepper_lidar_control.ino`

**Hardware Requirements**:
- Arduino board (Uno, Nano, Mega, etc.)
- Stepper motor driver (e.g., DM542, TB6600)
- Stepper motor (configured for 6400 steps/revolution)

**Pin Configuration**:
- `PUL` (Pin 2): Pulse output to driver
- `DIR` (Pin 3): Direction control
- `ENA` (Pin 4): Enable/disable driver

**Communication Protocol**:
- **Baud Rate**: 115200
- **Command Format**: `cmd:<angle_in_radians>\n`
- **State Output**: `angle:<current_angle_in_radians>\n` (published every 50ms)

**Example Commands**:
```
cmd:0.0          # Move to 0 radians (0°)
cmd:1.5708       # Move to π/2 radians (90°)
cmd:3.14159      # Move to π radians (180°)
cmd:6.28318      # Move to 2π radians (360°/0°)
```

**Features**:
- Timer-based ISR for smooth stepping
- Serial communication for command and state
- Automatic angle normalization (0 to 2π)
- Non-blocking operation

### 2. Hardware Interface Plugin

**Files**: 
- `include/reccobot_lidar_control/arduino_lidar_hardware.hpp`
- `src/arduino_lidar_hardware.cpp`

**Description**: 
Implements `hardware_interface::SystemInterface` for ros2_control integration. Communicates with Arduino via Serial/TCP.

**URDF Configuration**:
```xml
<ros2_control name="ArduinoLidarSystem" type="system">
  <hardware>
    <plugin>reccobot_lidar_control/ArduinoLidarHardware</plugin>
    <param name="serial_port">/dev/ttyUSB0</param>
    <param name="baud_rate">115200</param>
  </hardware>
  <joint name="lidar_rotation_joint">
    <command_interface name="position"/>
    <state_interface name="position"/>
    <state_interface name="velocity"/>
  </joint>
</ros2_control>
```

### 3. LiDAR Position Controller

**Files**:
- `include/reccobot_lidar_control/lidar_position_controller.hpp`
- `src/lidar_position_controller.cpp`

**Description**:
Custom ros2_control controller for LiDAR positioning with automatic scanning capability.

**Features**:
- **Manual Positioning**: Send target angle via topic
- **Continuous Scanning**: Automatic back-and-forth scanning
- **Configurable Scan Speed**: Adjust scanning velocity
- **Scan Range Control**: Define min/max angles
- **Command Timeout**: Automatic fallback behavior

**Parameters**:
```yaml
lidar_position_controller:
  ros__parameters:
    joint: "lidar_rotation_joint"
    command_timeout: 0.5          # seconds
    continuous_scan: true         # Enable auto-scanning
    scan_speed: 1.0               # rad/s
    scan_min_angle: 0.0           # radians
    scan_max_angle: 6.28318530718 # radians (2π)
```

**Topics**:
- **Command** (Subscribed): `/lidar_position_controller/command` (std_msgs/Float64)
  - Publish target angle in radians
  - Resets scan pattern when received
  
- **State** (Published): `/lidar_position_controller/state` (std_msgs/Float64)
  - Current LiDAR angle in radians
  - Published at controller update rate (typically 100 Hz)

## Building

```bash
cd ~/reccobot_ws
colcon build --packages-select reccobot_lidar_control
source install/setup.bash
```

## Usage

### 1. Upload Arduino Firmware

```bash
# Using Arduino IDE
# Open: ~/reccobot_ws/src/reccobot_lidar_control/arduino/stepper_lidar_control/stepper_lidar_control.ino
# Select your board and port
# Click Upload

# Or using arduino-cli
arduino-cli compile --fqbn arduino:avr:nano ~/reccobot_ws/src/reccobot_lidar_control/arduino/stepper_lidar_control
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:nano ~/reccobot_ws/src/reccobot_lidar_control/arduino/stepper_lidar_control
```

### 2. Launch Complete Control System

```bash
ros2 launch reccobot_lidar_control lidar_control.launch.py
```

This launches:
- Controller manager with hardware interface
- Robot state publisher
- LiDAR position controller

### 3. Spawn Controller Only

If controller manager is already running:

```bash
ros2 launch reccobot_lidar_control lidar_controller_spawner.launch.py
```

**With custom parameters**:
```bash
ros2 launch reccobot_lidar_control lidar_controller_spawner.launch.py \
    continuous_scan:=false \
    scan_speed:=2.0
```

### 4. Manual Control

**Send position commands**:
```bash
# Move to 90 degrees (π/2 radians)
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 1.5708" --once

# Move to 180 degrees (π radians)
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 3.14159" --once

# Move to 270 degrees (3π/2 radians)
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 4.71239" --once
```

**Monitor current position**:
```bash
ros2 topic echo /lidar_position_controller/state
```

### 5. Check Controller Status

```bash
# List loaded controllers
ros2 control list_controllers

# View controller info
ros2 control list_hardware_interfaces
```

## Configuration

### Continuous Scanning Mode

Edit `config/lidar_controller.yaml`:

```yaml
lidar_position_controller:
  ros__parameters:
    continuous_scan: true      # Enable/disable
    scan_speed: 1.0            # Speed in rad/s
    scan_min_angle: 0.0        # Start angle
    scan_max_angle: 6.28318    # End angle (2π)
```

**Scan Behavior**:
- When `continuous_scan: true` and no manual commands received
- Sweeps from `scan_min_angle` to `scan_max_angle`
- Reverses direction at boundaries
- Smooth motion at `scan_speed` velocity

### Manual Position Mode

```yaml
lidar_position_controller:
  ros__parameters:
    continuous_scan: false     # Disable auto-scan
    command_timeout: 0.5       # Hold position if no command
```

## Troubleshooting

### Arduino Communication Issues

1. **Check serial port**:
   ```bash
   ls -l /dev/ttyUSB* /dev/ttyACM*
   ```

2. **Test serial connection**:
   ```bash
   # Install screen if needed: sudo apt install screen
   screen /dev/ttyUSB0 115200
   # Type: cmd:0.0
   # Should see: OK rad=0.000000
   # Press Ctrl+A then K to exit
   ```

3. **Check permissions**:
   ```bash
   sudo usermod -a -G dialout $USER
   # Logout and login again
   ```

### Controller Not Loading

1. **Verify plugin is exported**:
   ```bash
   ros2 pkg prefix reccobot_lidar_control
   cat $(ros2 pkg prefix reccobot_lidar_control)/share/reccobot_lidar_control/lidar_controller_plugin.xml
   ```

2. **Check controller manager logs**:
   ```bash
   ros2 run controller_manager ros2_control_node --ros-args --log-level debug
   ```

3. **List available controller types**:
   ```bash
   ros2 control list_controller_types | grep -i lidar
   ```

### Stepper Motor Issues

1. **No movement**:
   - Check ENA pin (should be LOW to enable)
   - Verify driver power supply
   - Check wiring between Arduino and driver
   - Ensure driver current limit is set correctly

2. **Jerky movement**:
   - Adjust `stepPeriodUs` in Arduino code (lower = faster, but may skip steps)
   - Check motor current settings
   - Verify mechanical load is not excessive

3. **Wrong direction**:
   - Swap motor winding connections
   - Or invert in code: `digitalWrite(DIR, !(targetStep > currentStep));`

## Advanced Usage

### Custom Scan Patterns

Modify `lidar_position_controller.cpp` update() method to implement:
- Spiral patterns
- Variable speed scanning
- ROI-focused scanning
- Synchronized scanning with other sensors

### Integration with Mapping

```python
#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64
from sensor_msgs.msg import LaserScan
import math

class LidarMapper(Node):
    def __init__(self):
        super().__init__('lidar_mapper')
        
        # Subscribe to LiDAR angle and scan data
        self.angle_sub = self.create_subscription(
            Float64, '/lidar_position_controller/state',
            self.angle_callback, 10)
        
        self.scan_sub = self.create_subscription(
            LaserScan, '/scan',
            self.scan_callback, 10)
        
        self.current_angle = 0.0
    
    def angle_callback(self, msg):
        self.current_angle = msg.data
    
    def scan_callback(self, msg):
        # Process scan with current rotation angle
        self.get_logger().info(f'Received scan at angle: {self.current_angle:.3f} rad')
        # Add your mapping logic here

if __name__ == '__main__':
    rclpy.init()
    node = LidarMapper()
    rclpy.spin(node)
    rclpy.shutdown()
```

## Testing

### Unit Tests

```bash
colcon test --packages-select reccobot_lidar_control
colcon test-result --verbose
```

### Hardware-in-the-Loop Testing

```bash
# Terminal 1: Launch hardware interface
ros2 launch reccobot_lidar_control lidar_control.launch.py

# Terminal 2: Send test positions
for angle in 0.0 1.5708 3.14159 4.71239; do
    ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: $angle" --once
    sleep 2
done

# Terminal 3: Monitor state
ros2 topic echo /lidar_position_controller/state
```

## Performance

- **Update Rate**: 100 Hz (configurable in controller_manager)
- **Position Resolution**: 6400 steps/rev = 0.00098 rad/step ≈ 0.056°/step
- **Maximum Scan Speed**: Limited by stepper motor and driver (typically 1-5 rad/s)
- **Serial Latency**: ~50ms state update rate from Arduino

## Dependencies

- ROS 2 (Humble/Iron/Rolling)
- ros2_control
- controller_interface
- realtime_tools
- hardware_interface
- Arduino (with TimerOne library)

## License

[Your License Here]

## Contributors

- [Your Name]
- [Other Contributors]

## References

- [ros2_control documentation](https://control.ros.org/)
- [Writing a new controller](https://control.ros.org/master/doc/ros2_controllers/doc/writing_new_controller.html)
- [Arduino TimerOne library](https://www.arduino.cc/reference/en/libraries/timerone/)
