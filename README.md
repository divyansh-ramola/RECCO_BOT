# RECCO_BOT

A ROS 2 workspace containing the RECCO_BOT robot project, including robot description files, URDF models, and associated submodules for autonomous navigation and perception.

## Table of Contents

- [Overview](#overview)
- [Repository Structure](#repository-structure)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Building the Workspace](#building-the-workspace)
- [Running the Robot](#running-the-robot)
- [Packages](#packages)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)

---

## Overview

This workspace contains the complete software stack for the RECCO_BOT, a quadruped robot designed for reconnaissance and exploration tasks. The project includes robot description files, visualization tools, and integration with various sensors including LiDAR and cameras.

---

## Repository Structure

```
reccobot_ws/
├── src/
│   ├── reccobot_description/    # Robot URDF models and meshes
│   ├── reccobot_state_machine/  # State machine implementation
│   ├── Person_MinkUNet/         # Person detection submodule
│   └── README.md                # This file
```

---

## Prerequisites

Before building this workspace, ensure you have the following installed:

- **Ubuntu 22.04** (Jammy Jellyfish) or compatible
- **ROS 2 Humble** (or your target ROS 2 distribution)
- **Python 3.10+**
- **Colcon build tools**

### Install ROS 2 Dependencies

```bash
sudo apt update
sudo apt install -y \
    ros-${ROS_DISTRO}-robot-state-publisher \
    ros-${ROS_DISTRO}-joint-state-publisher \
    ros-${ROS_DISTRO}-joint-state-publisher-gui \
    ros-${ROS_DISTRO}-rviz2 \
    ros-${ROS_DISTRO}-xacro \
    python3-colcon-common-extensions
```

---

## Installation

### Clone the Repository

Always clone this repository with its submodules to ensure all dependencies are initialized correctly:

```bash
git clone --recurse-submodules https://github.com/divyansh-ramola/RECCO_BOT.git
cd RECCO_BOT
```

### If You Forgot the --recurse-submodules Flag

If you already cloned the repository without submodules, initialize them with:

```bash
git submodule update --init --recursive
```

---

## Building the Workspace

Navigate to the workspace root and build all packages:

```bash
cd ~/reccobot_ws
colcon build
```

To build a specific package:

```bash
colcon build --packages-select reccobot_description
```

After building, source the workspace:

```bash
source install/setup.bash
```

**Note:** Add this line to your `~/.bashrc` for automatic sourcing:

```bash
echo "source ~/reccobot_ws/install/setup.bash" >> ~/.bashrc
```

---

## Running the Robot

### Launch Robot Visualization

To visualize the robot model in RViz with TF frames:

```bash
source ~/reccobot_ws/install/setup.bash
ros2 launch reccobot_description display.launch.py
```

This will launch:
- Robot State Publisher (publishes robot model and TF transforms)
- Joint State Publisher GUI (control joint positions)
- RViz2 (visualization with pre-configured display settings)

### Verify Package Installation

To verify that packages are correctly installed:

```bash
ros2 pkg list | grep reccobot
```

Expected output:
```
reccobot_description
reccobot_state_machine
```

### Check Available Launch Files

```bash
ros2 launch reccobot_description --show-args
```

---

## Packages

### reccobot_description

Contains the URDF robot model, mesh files, and visualization launch files. The robot model includes:
- 4-legged quadruped base with 12 degrees of freedom
- LiDAR sensor mount with rotation capability
- Camera system with pan-tilt mechanism
- Battery and electronic component models

See the [reccobot_description README](reccobot_description/README.md) for detailed information.

### reccobot_state_machine

State machine implementation for robot behavior control and task execution.

### Person_MinkUNet

Submodule for person detection using MinkUNet architecture with 3D point cloud data.

---

## Troubleshooting

### Issue: Package not found after building

**Solution:** Ensure you have sourced the workspace setup file:

```bash
source ~/reccobot_ws/install/setup.bash
```

### Issue: Missing dependencies during build

**Solution:** Install missing ROS 2 dependencies:

```bash
rosdep install --from-paths src --ignore-src -r -y
```

### Issue: Submodules not initialized

**Solution:** Initialize and update all submodules:

```bash
git submodule update --init --recursive
```

### Issue: RViz does not display robot model

**Solution:** Verify that robot_state_publisher is running:

```bash
ros2 node list
ros2 topic echo /robot_description
```

---

## Contributing

Contributions are welcome. Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Commit your changes with clear messages
4. Push to your branch (`git push origin feature/your-feature`)
5. Open a Pull Request

---

## License

This project is licensed under the terms specified in the LICENSE file.

---

## Contact

For questions or support, please open an issue on the GitHub repository.

**Repository:** [https://github.com/divyansh-ramola/RECCO_BOT](https://github.com/divyansh-ramola/RECCO_BOT)