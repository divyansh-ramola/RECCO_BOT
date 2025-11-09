# Reccobot Lidar Control

This package provides a node to continuously rotate the lidar for 3D mapping.

## Overview

The `lidar_rotation_node` controls the `base_coupling` joint which rotates the lidar sensor between -360° and +360° (approximately -6.28 to 6.25 radians).

## Usage

### 1. Launch Gazebo with the robot:
```bash
source install/setup.bash
ros2 launch reccobot_description gazebo.launch.py
```

### 2. In a new terminal, launch the lidar rotation node:
```bash
source install/setup.bash
ros2 launch reccobot_lidar_control lidar_rotation.launch.py
```

### 3. Adjust rotation speed (optional):
You can modify the parameters in the launch file or pass them as arguments:
```bash
ros2 launch reccobot_lidar_control lidar_rotation.launch.py rotation_speed:=1.0
```

## Parameters

- `rotation_speed` (default: 0.5): Rotation speed in radians per second
- `min_angle` (default: -6.28): Minimum rotation angle in radians
- `max_angle` (default: 6.25): Maximum rotation angle in radians

## How it works

1. The node publishes position commands to `/lidar_controller/commands` topic
2. The lidar rotates back and forth between min and max angles
3. The vertical lidar scan combined with rotation creates a 3D point cloud
4. Direction reverses automatically when limits are reached

## Testing

Check if the controller is receiving commands:
```bash
ros2 topic echo /lidar_controller/commands
```

Monitor the lidar joint position:
```bash
ros2 topic echo /joint_states | grep -A 5 base_coupling
```

## 3D Mapping

To collect 3D scan data, you can subscribe to the `/scan` topic and combine it with the TF transform of the rotating lidar link to build a 3D point cloud.

The scan data is published in the `lidarl_sensor` frame which rotates with the lidar, so you can use `tf2` to transform the 2D scans into the fixed `base_link` frame over time to create a complete 3D map.
