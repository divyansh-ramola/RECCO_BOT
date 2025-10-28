# reccobot_description

A ROS 2 package containing the complete robot description for RECCO_BOT, including URDF files, 3D mesh models, and visualization configurations for simulation and display in RViz.

## Table of Contents

- [Overview](#overview)
- [Package Contents](#package-contents)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Modifying Robot Orientation](#modifying-robot-orientation)
- [Online Visualization](#online-visualization)
- [File Structure](#file-structure)
- [Technical Details](#technical-details)

---

## Overview

This package defines the complete kinematic and visual description of the RECCO_BOT quadruped robot. It includes:

- **URDF Model**: Complete robot description with 12 DOF leg system
- **3D Meshes**: High-quality STL mesh files for all robot components
- **Launch Files**: Pre-configured launch files for visualization
- **RViz Configuration**: Custom RViz setup with TF frame display

The robot features a world-fixed reference frame with adjustable orientation, making it suitable for both visualization and simulation environments.

---

## Package Contents

```
reccobot_description/
├── CMakeLists.txt              # Build configuration
├── package.xml                 # Package metadata and dependencies
├── README.md                   # This file
├── config/
│   └── joint_names_reccobot_description.yaml
├── launch/
│   ├── display.launch          # ROS 1 launch file (legacy)
│   ├── display.launch.py       # ROS 2 Python launch file
│   ├── gazebo.launch           # Gazebo simulation launch (ROS 1)
│   └── gazebo.launch.py        # Gazebo simulation launch (ROS 2)
├── meshes/                     # STL mesh files for all robot links
│   ├── base_link.STL
│   ├── servo*.STL
│   ├── coxa*.STL
│   ├── femur*.STL
│   ├── tibia*.STL
│   ├── feet*.STL
│   ├── lidar*.STL
│   ├── cam*.STL
│   └── ...
├── urdf/
│   ├── reccobot_description.urdf    # Main robot description
│   ├── reccobot_gazebo.urdf         # Gazebo-specific URDF
│   └── reccobot_ros2_control.urdf   # ROS 2 Control configuration
└── urdf.rviz                   # RViz configuration file
```

---

## Prerequisites

### Required ROS 2 Packages

```bash
sudo apt update
sudo apt install -y \
    ros-${ROS_DISTRO}-robot-state-publisher \
    ros-${ROS_DISTRO}-joint-state-publisher \
    ros-${ROS_DISTRO}-joint-state-publisher-gui \
    ros-${ROS_DISTRO}-rviz2 \
    ros-${ROS_DISTRO}-xacro
```

### System Requirements

- ROS 2 Humble or later
- Ubuntu 22.04 or compatible Linux distribution
- OpenGL-capable graphics card (for RViz visualization)

---

## Installation

### Build the Package

Navigate to your workspace and build:

```bash
cd ~/reccobot_ws
colcon build --packages-select reccobot_description
source install/setup.bash
```

### Verify Installation

Check that the package is correctly installed:

```bash
ros2 pkg prefix reccobot_description
```

Expected output:
```
/home/<username>/reccobot_ws/install/reccobot_description
```

---

## Usage

### Launch Robot Visualization with RViz

To visualize the robot model with TF frames and joint control:

```bash
ros2 launch reccobot_description display.launch.py
```

This will automatically:
1. Load the URDF model into the ROS parameter server
2. Start the Robot State Publisher (publishes TF transforms)
3. Launch Joint State Publisher GUI (control joint positions interactively)
4. Open RViz2 with pre-configured display settings showing:
   - Robot model visualization
   - TF frame axes and labels
   - Grid reference plane

### View Available Launch Arguments

```bash
ros2 launch reccobot_description display.launch.py --show-args
```

### Publish Robot Description Only

To only publish the robot description without visualization:

```bash
ros2 run robot_state_publisher robot_state_publisher \
    --ros-args -p robot_description:="$(cat ~/reccobot_ws/install/reccobot_description/share/reccobot_description/urdf/reccobot_description.urdf)"
```

### Verify TF Transforms

Check that all transforms are being published:

```bash
ros2 run tf2_ros tf2_echo world base_link
```

View the complete TF tree:

```bash
ros2 run tf2_tools view_frames
```

This generates a PDF file (`frames.pdf`) showing all coordinate frame relationships.

---

## Modifying Robot Orientation

The robot orientation relative to the world frame can be easily adjusted by modifying the URDF file.

### Location of Rotation Parameters

Open the URDF file:

```bash
nano ~/reccobot_ws/src/reccobot_description/urdf/reccobot_description.urdf
```

Find the commented section near the top of the file:

```xml
<!-- 
  ROTATION ADJUSTMENT: 
  To modify the robot's orientation in the world frame, change the 'rpy' values below.
  Current: rpy="0 0 1.5708" rotates the robot 90 degrees around Z-axis
  Format: rpy="roll pitch yaw" in radians
  Common values: 
    - 90 degrees = 1.5708 radians
    - 180 degrees = 3.14159 radians
    - -90 degrees = -1.5708 radians
-->
<joint name="world_to_base" type="fixed">
  <parent link="world"/>
  <child link="base_link"/>
  <origin xyz="0 0 0" rpy="0 0 1.5708"/>
</joint>
```

### Common Rotation Examples

- **No rotation**: `rpy="0 0 0"`
- **90° clockwise around Z**: `rpy="0 0 -1.5708"`
- **180° around Z**: `rpy="0 0 3.14159"`
- **Upside down**: `rpy="3.14159 0 0"`

After modifying, rebuild the package:

```bash
cd ~/reccobot_ws
colcon build --packages-select reccobot_description
source install/setup.bash
```

---

## Online Visualization

To visualize the URDF without installing ROS locally:

### Using URDF Viewer Web Tool

1. Visit [http://urdf.robotsfan.com/](http://urdf.robotsfan.com/)
2. Drag and drop the entire `reccobot_description` folder
3. Ensure mesh paths use relative references (e.g., `package://reccobot_description/meshes/...`)

### Using Gazebo Sim Online

1. Visit [Gazebo Web](https://app.gazebosim.org/)
2. Upload the URDF file
3. Configure mesh path mappings if necessary

**Note:** Mesh paths in the URDF use ROS package URLs (`package://`), which may need conversion to relative paths for some online viewers.

---

## File Structure

### URDF Files

- **reccobot_description.urdf**: Main robot description with complete kinematic chain and visual/collision meshes
- **reccobot_gazebo.urdf**: Extended URDF with Gazebo-specific plugins and sensors
- **reccobot_ros2_control.urdf**: URDF with ROS 2 Control hardware interfaces

### Launch Files

- **display.launch.py**: Primary visualization launch file (ROS 2)
- **gazebo.launch.py**: Gazebo simulation environment launch (ROS 2)

### Configuration Files

- **urdf.rviz**: Pre-configured RViz display with robot model and TF visualization
- **joint_names_reccobot_description.yaml**: Joint name mappings for control interfaces

---

## Technical Details

### Robot Specifications

- **Type**: Quadruped mobile robot
- **Degrees of Freedom**: 12 (3 per leg)
- **Leg Configuration**: 4 legs with coxa-femur-tibia joints
- **Additional DOF**: 
  - LiDAR rotation: 1 DOF
  - Camera pan-tilt: 2 DOF
- **Sensors**:
  - LiDAR scanner
  - RGB Camera with pan-tilt mechanism
  - IMU (Inertial Measurement Unit)

### Coordinate Frames

- **world**: Fixed world reference frame
- **base_link**: Robot body center
- **leg frames**: coxa, femur, tibia, feet (for each of 4 legs)
- **sensor frames**: lidar, camera, IMU

### Joint Types

- **Revolute joints**: All leg joints with specified position limits
- **Fixed joints**: Sensor mounts and structural connections
- **Continuous joints**: LiDAR rotation (unlimited rotation)

---

## Troubleshooting

### Issue: Robot appears sideways in RViz

**Solution:** The robot orientation is controlled by the `world_to_base` joint. Modify the `rpy` parameter as described in the [Modifying Robot Orientation](#modifying-robot-orientation) section.

### Issue: Meshes not loading in RViz

**Solution:** Verify mesh file paths and ensure the package is properly sourced:

```bash
source ~/reccobot_ws/install/setup.bash
echo $AMENT_PREFIX_PATH
```

### Issue: TF frames not visible

**Solution:** Enable TF display in RViz:
1. In RViz, click "Add" in the Displays panel
2. Select "TF" from the list
3. Adjust marker scale if frames are too small

### Issue: Joint State Publisher GUI not appearing

**Solution:** Ensure the package is installed:

```bash
sudo apt install ros-${ROS_DISTRO}-joint-state-publisher-gui
```

---

## Contributing

When contributing modifications to the robot description:

1. Test changes in RViz before committing
2. Verify TF tree consistency with `view_frames`
3. Ensure mesh files are properly referenced
4. Update this README if adding new features
5. Validate URDF syntax: `check_urdf reccobot_description.urdf`

---

## License

This package is part of the RECCO_BOT project. See the main repository LICENSE file for details.

---

## Support

For issues specific to this package:
- Open an issue on the [GitHub repository](https://github.com/divyansh-ramola/RECCO_BOT)
- Include ROS 2 version, Ubuntu version, and error messages
- Attach relevant log files from `~/.ros/log/`
