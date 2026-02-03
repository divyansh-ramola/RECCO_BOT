# Reccobot Lidar Control

This package provides hardware integration for Arduino-controlled lidar rotation for the Reccobot quadruped robot.

## Overview

The package implements a ros2_control hardware interface plugin that communicates with an Arduino controller to rotate a 2D lidar, enabling 3D point cloud generation.

### Components

1. **Arduino Hardware Plugin** (`ArduinoLidarHardware`)
   - Full ros2_control `SystemInterface` implementation
   - TCP socket communication with Arduino (192.168.1.150)
   - Controls `base_coupling` joint (lidar rotation)

2. **Arduino Firmware** (`arduino/2/2.ino`)
   - TCP state server (port 8000): publishes `angle:<radians>\n`
   - TCP command server (port 8001): receives `cmd:<radians>\n`
   - Stepper motor control with position feedback

3. **Point Cloud Accumulator** (`lidar_to_cloud`)
   - Accumulates 2D laser scans over rotation cycle
   - Uses TF to transform scans into 3D space
   - Publishes complete 3D point clouds to `/lidar_3d`

## Hardware vs Simulation

**Simulation Mode** (`sim_mode:=true`):
- All joints controlled by Gazebo
- Launch: `ros2 launch reccobot_description gazebo.launch.py`

**Hardware Mode** (`sim_mode:=false`):
- Legs/camera use mock hardware (placeholder)
- Lidar uses Arduino plugin for real rotation
- Launch: `ros2 launch reccobot_controllers controller.launch.py is_sim:=false`

## Hardware Setup

### Arduino Configuration

1. **Network**: Static IP 192.168.1.150, ports 8000 (state) / 8001 (command)
2. **Upload**: `arduino/2/2.ino` (requires UIPEthernet library)
3. **Hardware**: Stepper motor + Ethernet shield + rotating lidar mount

## Hardware Setup

### Arduino Configuration

1. **Network**: Static IP 192.168.1.150, ports 8000 (state) / 8001 (command)
2. **Upload**: `arduino/2/2.ino` (requires UIPEthernet library)
3. **Hardware**: Stepper motor + Ethernet shield + rotating lidar mount

## Usage

### Launch Hardware System

```bash
# Source workspace
source install/setup.bash

# Launch ros2_control with hardware plugin
ros2 launch reccobot_controllers controller.launch.py is_sim:=false

# In separate terminals:
# 1. Launch lidar driver
ros2 launch ldlidar ldlidar_bringup.launch.py

# 2. Launch rotation controller and point cloud accumulator
ros2 launch reccobot_lidar_control lidar_rotation.launch.py sim_mode:=false
```

### Launch Simulation

```bash
# Single command for full simulation
ros2 launch reccobot_description gazebo.launch.py
```

### Test Hardware Communication

```bash
# Verify plugin is loaded
ros2 control list_hardware_interfaces
# Should show: base_coupling/position [available] [claimed]

# Command lidar rotation
ros2 topic pub /lidar_controller/commands std_msgs/msg/Float64MultiArray "{data: [1.57]}" --once

# Monitor joint states
ros2 topic echo /joint_states | grep base_coupling

# Check Arduino connection directly
nc 192.168.1.150 8000  # Should show angle updates
```

### View Point Cloud

```bash
rviz2
# Add PointCloud2 display
# Topic: /lidar_3d
# Fixed Frame: base_link
```

## Parameters

### Lidar Rotation Node
- `rotation_speed` (default: 0.5): Rotation speed in radians/second
- `min_angle` (default: -6.28): Minimum rotation angle
- `max_angle` (default: 6.25): Maximum rotation angle

### Hardware Plugin (configured in URDF)
- `ip`: Arduino IP address (default: 192.168.1.150)
- `state_port`: TCP port for state updates (default: 8000)
- `command_port`: TCP port for commands (default: 8001)

## Troubleshooting

**Plugin Not Found**: Verify build completed and plugin is exported
```bash
ros2 pkg xml reccobot_lidar_control | grep hardware_interface
ls install/reccobot_lidar_control/lib/libarduino_lidar_hardware.so
```

**Connection Refused**: Check Arduino network and firmware
```bash
ping 192.168.1.150
nc 192.168.1.150 8000  # Should show angle updates
```

**No Point Cloud**: Verify lidar is publishing and rotating
```bash
ros2 topic hz /scan
ros2 topic echo /joint_states | grep base_coupling
ros2 run tf2_tools view_frames  # Check TF tree
```

## Architecture Details

The hardware plugin implements `hardware_interface::SystemInterface`:
- **on_activate()**: Establishes TCP connections to Arduino
- **read()**: Non-blocking read of current angle from port 8000
- **write()**: Sends target angle to port 8001
- **on_deactivate()**: Closes connections

Communication protocol:
- Arduino publishes: `angle:<radians>\n` (e.g., `angle:1.57`)
- Plugin sends: `cmd:<radians>\n` (e.g., `cmd:3.14`)

The plugin integrates seamlessly with ros2_control, allowing the lidar joint to be controlled via standard controllers while communicating with real hardware.

ros2 topic echo /joint_states | grep -A 5 base_coupling
```

## 3D Mapping

To collect 3D scan data, you can subscribe to the `/scan` topic and combine it with the TF transform of the rotating lidar link to build a 3D point cloud.

The scan data is published in the `lidarl_sensor` frame which rotates with the lidar, so you can use `tf2` to transform the 2D scans into the fixed `base_link` frame over time to create a complete 3D map.
