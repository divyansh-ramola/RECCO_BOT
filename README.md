# RECCOBOT - Quadruped Robot with 3D Mapping and Deep RL



<div align="center">



![LiDAR 3D Mapping](images/lidar_mapping.gif)<div align="center">A ROS 2 workspace containing the RECCO_BOT robot project, including robot description files, URDF models, and associated submodules for autonomous navigation and perception.



*Real-time 3D mapping using rotating 2D LiDAR*



![Gazebo Simulation](images/movemet.gif)

## Table of Contents



*Autonomous navigation in Gazebo simulation*



</div>*Real-time 3D mapping using rotating 2D LiDAR*



A ROS 2 workspace containing the RECCOBOT project - a versatile quadruped robot platform designed for reconnaissance, exploration, and autonomous navigation tasks. Features include 3D mapping using rotating 2D LiDAR, deep reinforcement learning for locomotion, ROS 2 control integration, and Gazebo simulation.- [Repository Structure](#repository-structure)



## 🌟 Key Features



- **Quadruped Robot Design**: 12 DOF leg system with 3 joints per leg (coxa, femur, tibia)- [Installation](#installation)

- **3D Mapping**: Rotating 2D LiDAR creates 3D point clouds for environment mapping

- **Deep Reinforcement Learning**: PPO-based training for quadruped locomotion*Autonomous navigation in Gazebo simulation*- [Building the Workspace](#building-the-workspace)

- **ROS 2 Control Integration**: Hardware-agnostic control using ros2_control framework

- **Gazebo Simulation**: Full robot simulation with physics and sensor plugins- [Running the Robot](#running-the-robot)

- **Sensor Suite**: LiDAR, RGB camera with pan-tilt, IMU

- **State Machine**: Behavior coordination and task management</div>- [Packages](#packages)

- **Real-time Visualization**: RViz2 with custom configurations

- [Troubleshooting](#troubleshooting)

---

A ROS 2 workspace containing the RECCOBOT project - a versatile quadruped robot platform designed for reconnaissance, exploration, and autonomous navigation tasks. Features include 3D mapping using rotating 2D LiDAR, deep reinforcement learning for locomotion, ROS 2 control integration, and Gazebo simulation.- [Contributing](#contributing)

## 📋 Table of Contents



- [Overview](#overview)

- [Repository Structure](#repository-structure)## 🌟 Key Features---

- [Prerequisites](#prerequisites)

- [Installation](#installation)

- [Building the Workspace](#building-the-workspace)

- [Quick Start](#quick-start)- **Quadruped Robot Design**: 12 DOF leg system with 3 joints per leg (coxa, femur, tibia)## Overview

- [Packages](#packages)

- [Features in Detail](#features-in-detail)- **3D Mapping**: Rotating 2D LiDAR creates 3D point clouds for environment mapping

- [Training Deep RL Agent](#training-deep-rl-agent)

- [3D Mapping with LiDAR](#3d-mapping-with-lidar)- **Deep Reinforcement Learning**: PPO-based training for quadruped locomotionThis workspace contains the complete software stack for the RECCO_BOT, a quadruped robot designed for reconnaissance and exploration tasks. The project includes robot description files, visualization tools, and integration with various sensors including LiDAR and cameras.

- [Troubleshooting](#troubleshooting)

- [Contributing](#contributing)- **ROS 2 Control Integration**: Hardware-agnostic control using ros2_control framework



---- **Gazebo Simulation**: Full robot simulation with physics and sensor plugins---



## 🤖 Overview- **Sensor Suite**: LiDAR, RGB camera with pan-tilt, IMU



RECCOBOT is a comprehensive quadruped robotics platform built on ROS 2 Humble. The project demonstrates advanced robotics concepts including:- **State Machine**: Behavior coordination and task management## Repository Structure



- **Kinematics & Dynamics**: Full URDF model with accurate mass properties and inertia- **Real-time Visualization**: RViz2 with custom configurations

- **Sensor Fusion**: Integration of LiDAR, camera, and IMU data

- **Autonomous Navigation**: Deep RL-trained walking behaviors```

- **3D Perception**: Converting 2D LiDAR scans to 3D point clouds through rotation

- **Control Systems**: Position and velocity control using ros2_control---reccobot_ws/

- **Simulation to Reality**: Gazebo-based development with real hardware deployment path

├── src/

---

## 📋 Table of Contents│   ├── reccobot_description/    # Robot URDF models and meshes

## 📁 Repository Structure

│   ├── reccobot_state_machine/  # State machine implementation

```

reccobot_ws/- [Overview](#overview)│   ├── Person_MinkUNet/         # Person detection submodule

├── src/

│   ├── reccobot_description/         # Robot URDF, meshes, launch files- [Repository Structure](#repository-structure)│   └── README.md                # This file

│   │   ├── urdf/                     # URDF robot descriptions

│   │   ├── meshes/                   # STL 3D models- [Prerequisites](#prerequisites)```

│   │   ├── launch/                   # Launch files for visualization & simulation

│   │   ├── scripts/                  # Deep RL training scripts- [Installation](#installation)

│   │   ├── worlds/                   # Gazebo world files

│   │   └── rviz/                     # RViz configurations- [Building the Workspace](#building-the-workspace)---

│   │

│   ├── reccobot_controllers/         # ROS 2 Control configuration- [Quick Start](#quick-start)

│   │   ├── config/                   # Controller parameters

│   │   └── launch/                   # Controller launch files- [Packages](#packages)## Prerequisites

│   │

│   ├── reccobot_lidar_control/       # LiDAR rotation for 3D mapping- [Features in Detail](#features-in-detail)

│   │   ├── src/                      # C++ node for lidar rotation

│   │   └── launch/                   # LiDAR control launch files- [Training Deep RL Agent](#training-deep-rl-agent)Before building this workspace, ensure you have the following installed:

│   │

│   ├── reccobot_state_machine/       # Behavior state machine- [3D Mapping with LiDAR](#3d-mapping-with-lidar)

│   │   └── reccobot_state_machine/   # Python state machine implementation

│   │- [Troubleshooting](#troubleshooting)- **Ubuntu 22.04** (Jammy Jellyfish) or compatible

│   └── images/                       # Documentation images and GIFs

│- [Contributing](#contributing)- **ROS 2 Humble** (or your target ROS 2 distribution)

├── build/                            # Build artifacts

├── install/                          # Installed packages- **Python 3.10+**

└── log/                              # Build and runtime logs

```---- **Colcon build tools**



---



## 🔧 Prerequisites## 🤖 Overview### Install ROS 2 Dependencies



### System Requirements



- **OS**: Ubuntu 22.04 (Jammy Jellyfish) or compatibleRECCOBOT is a comprehensive quadruped robotics platform built on ROS 2 Humble. The project demonstrates advanced robotics concepts including:```bash

- **ROS 2**: Humble Hawksbill

- **Python**: 3.10+sudo apt update

- **Gazebo**: Fortress or Harmonic (for simulation)

- **Kinematics & Dynamics**: Full URDF model with accurate mass properties and inertiasudo apt install -y \

### Install ROS 2 Humble

- **Sensor Fusion**: Integration of LiDAR, camera, and IMU data    ros-${ROS_DISTRO}-robot-state-publisher \

If you haven't installed ROS 2 Humble yet:

- **Autonomous Navigation**: Deep RL-trained walking behaviors    ros-${ROS_DISTRO}-joint-state-publisher \

```bash

# Add ROS 2 repository- **3D Perception**: Converting 2D LiDAR scans to 3D point clouds through rotation    ros-${ROS_DISTRO}-joint-state-publisher-gui \

sudo apt update && sudo apt install -y software-properties-common

sudo add-apt-repository universe- **Control Systems**: Position and velocity control using ros2_control    ros-${ROS_DISTRO}-rviz2 \

sudo apt update && sudo apt install -y curl

sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg- **Simulation to Reality**: Gazebo-based development with real hardware deployment path    ros-${ROS_DISTRO}-xacro \

echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null

    python3-colcon-common-extensions

# Install ROS 2 Humble

sudo apt update---```

sudo apt install -y ros-humble-desktop

```



### Install Dependencies## 📁 Repository Structure---



```bash

sudo apt update

sudo apt install -y \```## Installation

    ros-humble-robot-state-publisher \

    ros-humble-joint-state-publisher \reccobot_ws/

    ros-humble-joint-state-publisher-gui \

    ros-humble-rviz2 \├── src/### Clone the Repository

    ros-humble-xacro \

    ros-humble-ros2-control \│   ├── reccobot_description/         # Robot URDF, meshes, launch files

    ros-humble-ros2-controllers \

    ros-humble-controller-manager \│   │   ├── urdf/                     # URDF robot descriptionsAlways clone this repository with its submodules to ensure all dependencies are initialized correctly:

    ros-humble-gazebo-ros-pkgs \

    ros-humble-gazebo-ros2-control \│   │   ├── meshes/                   # STL 3D models

    python3-colcon-common-extensions \

    python3-rosdep \│   │   ├── launch/                   # Launch files for visualization & simulation```bash

    python3-pip

│   │   ├── scripts/                  # Deep RL training scriptsgit clone --recurse-submodules https://github.com/divyansh-ramola/RECCO_BOT.git

# Deep RL dependencies (optional, for training)

pip3 install stable-baselines3 gymnasium torch tensorboard│   │   ├── worlds/                   # Gazebo world filescd RECCO_BOT

```

│   │   └── rviz/                     # RViz configurations```

### Initialize rosdep

│   │

```bash

sudo rosdep init│   ├── reccobot_controllers/         # ROS 2 Control configuration### If You Forgot the --recurse-submodules Flag

rosdep update

```│   │   ├── config/                   # Controller parameters



---│   │   └── launch/                   # Controller launch filesIf you already cloned the repository without submodules, initialize them with:



## 📥 Installation│   │



### Clone the Repository│   ├── reccobot_lidar_control/       # LiDAR rotation for 3D mapping```bash



```bash│   │   ├── src/                      # C++ node for lidar rotationgit submodule update --init --recursive

# Create workspace directory

mkdir -p ~/reccobot_ws/src│   │   └── launch/                   # LiDAR control launch files```

cd ~/reccobot_ws/src

│   │

# Clone the repository

git clone https://github.com/divyansh-ramola/RECCO_BOT.git .│   ├── reccobot_state_machine/       # Behavior state machine---

```

│   │   └── reccobot_state_machine/   # Python state machine implementation

### Install Package Dependencies

│   │## Building the Workspace

```bash

cd ~/reccobot_ws│   ├── ldrobot-lidar-ros2/           # LDRobot LiDAR driver

rosdep install --from-paths src --ignore-src -r -y

```│   │   └── ldlidar_node/             # ROS 2 node for LiDARNavigate to the workspace root and build all packages:



---│   │



## 🔨 Building the Workspace│   ├── articubot_one/                # Reference robot examples```bash



### Build All Packages│   ├── 3D-Mapping-Using-2D-LiDAR-ROS/ # 3D mapping algorithmscd ~/reccobot_ws



```bash│   ├── SpiderBot_DeepRL/             # Deep RL training frameworkcolcon build

cd ~/reccobot_ws

colcon build --symlink-install│   └── images/                       # Documentation images and GIFs```

```

│

### Build Specific Package

├── build/                            # Build artifactsTo build a specific package:

```bash

colcon build --packages-select reccobot_description├── install/                          # Installed packages

```

└── log/                              # Build and runtime logs```bash

### Source the Workspace

```colcon build --packages-select reccobot_description

```bash

source ~/reccobot_ws/install/setup.bash```

```

---

**Tip:** Add to your `~/.bashrc` for automatic sourcing:

After building, source the workspace:

```bash

echo "source ~/reccobot_ws/install/setup.bash" >> ~/.bashrc## 🔧 Prerequisites

source ~/.bashrc

``````bash



---### System Requirementssource install/setup.bash



## 🚀 Quick Start```



### 1. Visualize Robot in RViz- **OS**: Ubuntu 22.04 (Jammy Jellyfish) or compatible



View the robot model with interactive joint control:- **ROS 2**: Humble Hawksbill**Note:** Add this line to your `~/.bashrc` for automatic sourcing:



```bash- **Python**: 3.10+

ros2 launch reccobot_description display.launch.py

```- **Gazebo**: Fortress or Harmonic (for simulation)```bash



### 2. Launch Gazebo Simulationecho "source ~/reccobot_ws/install/setup.bash" >> ~/.bashrc



Start the robot in a simulated environment:### Install ROS 2 Humble```



```bash

ros2 launch reccobot_description gazebo.launch.py

```If you haven't installed ROS 2 Humble yet:---



### 3. Start 3D Mapping



In a new terminal, start the LiDAR rotation for 3D mapping:```bash## Running the Robot



```bash# Add ROS 2 repository

source ~/reccobot_ws/install/setup.bash

ros2 launch reccobot_lidar_control lidar_rotation.launch.pysudo apt update && sudo apt install -y software-properties-common### Launch Robot Visualization

```

sudo add-apt-repository universe

### 4. Control the Robot

sudo apt update && sudo apt install -y curlTo visualize the robot model in RViz with TF frames:

Use keyboard teleop or the trained RL agent to move the robot.

sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg

---

echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null```bash

## 📦 Packages

source ~/reccobot_ws/install/setup.bash

### reccobot_description

# Install ROS 2 Humbleros2 launch reccobot_description display.launch.py

**Complete robot description and simulation package**

sudo apt update```

- URDF robot models with accurate kinematics

- 3D mesh files for all robot componentssudo apt install -y ros-humble-desktop

- Gazebo simulation launch files

- RViz visualization configurations```This will launch:

- Deep RL training scripts

- Robot State Publisher (publishes robot model and TF transforms)

**Key Features:**

- 12 DOF quadruped with 4 legs### Install Dependencies- Joint State Publisher GUI (control joint positions)

- Rotating LiDAR mount (1 DOF)

- Camera pan-tilt mechanism (2 DOF)- RViz2 (visualization with pre-configured display settings)

- IMU sensor integration

- Physics-accurate collision meshes```bash



📖 [Detailed Documentation](reccobot_description/README.md)sudo apt update### Verify Package Installation



### reccobot_controllerssudo apt install -y \



**ROS 2 Control integration package**    ros-humble-robot-state-publisher \To verify that packages are correctly installed:



- Hardware-agnostic control interfaces    ros-humble-joint-state-publisher \

- Position and velocity controllers

- Joint trajectory controllers    ros-humble-joint-state-publisher-gui \```bash

- Controller configuration files

    ros-humble-rviz2 \ros2 pkg list | grep reccobot

**Supported Controllers:**

- Joint State Broadcaster    ros-humble-xacro \```

- Position Controllers (per joint)

- Forward Command Controller    ros-humble-ros2-control \

- Joint Trajectory Controller

    ros-humble-ros2-controllers \Expected output:

### reccobot_lidar_control

    ros-humble-controller-manager \```

**3D mapping through LiDAR rotation**

    ros-humble-gazebo-ros-pkgs \reccobot_description

- Continuous rotation of 2D LiDAR sensor

- Configurable rotation speed and range    ros-humble-gazebo-ros2-control \reccobot_state_machine

- Creates 3D point clouds from 2D scans

- Real-time TF frame updates    python3-colcon-common-extensions \```



**Features:**    python3-rosdep \

- Rotation range: -360° to +360°

- Adjustable speed: 0.1 to 2.0 rad/s    python3-pip### Check Available Launch Files

- Automatic direction reversal at limits

- Synchronized with TF transforms



📖 [Detailed Documentation](reccobot_lidar_control/README.md)# Deep RL dependencies (optional, for training)```bash



### reccobot_state_machinepip3 install stable-baselines3 gymnasium torch tensorboardros2 launch reccobot_description --show-args



**High-level behavior coordination**``````



- State machine for robot behaviors

- Task planning and execution

- Sensor-driven state transitions### Initialize rosdep---



---



## 🎯 Features in Detail```bash## Packages



### 3D Mapping Systemsudo rosdep init



The robot uses a rotating 2D LiDAR to create 3D point clouds:rosdep update### reccobot_description



1. **Hardware Setup**: 2D LiDAR mounted on a rotating servo```

2. **Rotation Control**: Continuous back-and-forth rotation (-360° to +360°)

3. **Data Collection**: 2D scans captured at different anglesContains the URDF robot model, mesh files, and visualization launch files. The robot model includes:

4. **3D Reconstruction**: TF transforms combine scans into 3D space

5. **Visualization**: Real-time point cloud in RViz---- 4-legged quadruped base with 12 degrees of freedom



**Demo:**- LiDAR sensor mount with rotation capability

```bash

# Terminal 1: Launch Gazebo## 📥 Installation- Camera system with pan-tilt mechanism

ros2 launch reccobot_description gazebo.launch.py

- Battery and electronic component models

# Terminal 2: Start LiDAR rotation

ros2 launch reccobot_lidar_control lidar_rotation.launch.py### Clone the Repository



# Terminal 3: Visualize in RVizSee the [reccobot_description README](reccobot_description/README.md) for detailed information.

ros2 launch reccobot_description display.launch.py

``````bash



### Deep Reinforcement Learning# Create workspace directory### reccobot_state_machine



Train the quadruped to walk using PPO:mkdir -p ~/reccobot_ws/src



**Architecture:**cd ~/reccobot_ws/srcState machine implementation for robot behavior control and task execution.

- **State Space**: Joint positions, velocities, body orientation, target velocity

- **Action Space**: Joint position commands (12 dimensions)

- **Reward Function**: Forward velocity, stability, energy efficiency

- **Algorithm**: PPO with MLP policy# Clone the repository### Person_MinkUNet



**Training Process:**git clone https://github.com/divyansh-ramola/RECCO_BOT.git .

```bash

# Start Gazebo simulation```Submodule for person detection using MinkUNet architecture with 3D point cloud data.

ros2 launch reccobot_description gazebo.launch.py



# In another terminal, start training

cd ~/reccobot_ws/src/reccobot_description/scripts### Install Package Dependencies---

python3 train_walking.py

```



**Monitor Training:**```bash## Troubleshooting

```bash

tensorboard --logdir logs/cd ~/reccobot_ws

```

rosdep install --from-paths src --ignore-src -r -y### Issue: Package not found after building

### ROS 2 Control Integration

```

Hardware-agnostic control using ros2_control:

**Solution:** Ensure you have sourced the workspace setup file:

- **Joint State Interface**: Reads joint positions and velocities

- **Position Command Interface**: Sends position commands to joints---

- **Controller Manager**: Manages multiple controllers

- **Gazebo Integration**: Simulated hardware interface```bash



**Launch Controllers:**## 🔨 Building the Workspacesource ~/reccobot_ws/install/setup.bash

```bash

ros2 launch reccobot_controllers controller.launch.py```

```

### Build All Packages

**List Active Controllers:**

```bash### Issue: Missing dependencies during build

ros2 control list_controllers

``````bash



---cd ~/reccobot_ws**Solution:** Install missing ROS 2 dependencies:



## 🎓 Training Deep RL Agentcolcon build --symlink-install



### Training Script``````bash



The training script (`train_walking.py`) provides a complete RL training pipeline:rosdep install --from-paths src --ignore-src -r -y



**Features:**### Build Specific Package```

- Gymnasium-compatible environment

- PPO algorithm from stable-baselines3

- Automatic checkpointing

- Tensorboard logging```bash### Issue: Submodules not initialized

- Evaluation callbacks

colcon build --packages-select reccobot_description

### Custom Environment

```**Solution:** Initialize and update all submodules:

The `ReccobotEnv` environment provides:



```python

class ReccobotEnv(gymnasium.Env):### Source the Workspace```bash

    observation_space: Box(shape=(39,))  # Joint states + body pose + target

    action_space: Box(shape=(12,))       # Joint position commandsgit submodule update --init --recursive

    

    reward = (```bash```

        + forward_velocity_reward

        - energy_consumptionsource ~/reccobot_ws/install/setup.bash

        - deviation_from_upright

        + stability_bonus```### Issue: RViz does not display robot model

    )

```



### Training Parameters**Tip:** Add to your `~/.bashrc` for automatic sourcing:**Solution:** Verify that robot_state_publisher is running:



Modify training hyperparameters in `train_walking.py`:



```python```bash```bash

model = PPO(

    "MlpPolicy",echo "source ~/reccobot_ws/install/setup.bash" >> ~/.bashrcros2 node list

    env,

    learning_rate=3e-4,      # Learning ratesource ~/.bashrcros2 topic echo /robot_description

    n_steps=2048,            # Steps per update

    batch_size=64,           # Minibatch size``````

    n_epochs=10,             # Optimization epochs

    gamma=0.99,              # Discount factor

    clip_range=0.2,          # PPO clip range

)------

```



### Evaluation

## 🚀 Quick Start## Contributing

Test a trained model:



```python

python3 test_trained_model.py --model models/reccobot_walking_final.zip --episodes 10### 1. Visualize Robot in RVizContributions are welcome. Please follow these guidelines:

```



---

View the robot model with interactive joint control:1. Fork the repository

## 🗺️ 3D Mapping with LiDAR

2. Create a feature branch (`git checkout -b feature/your-feature`)

### How It Works

```bash3. Commit your changes with clear messages

1. **2D LiDAR Scan**: LD06 provides 360° 2D scan at 10Hz

2. **Rotation Mechanism**: Servo rotates LiDAR through vertical axisros2 launch reccobot_description display.launch.py4. Push to your branch (`git push origin feature/your-feature`)

3. **Frame Synchronization**: TF tree maintains lidar pose at each scan

4. **Point Cloud Assembly**: Scans transformed to fixed frame and accumulated```5. Open a Pull Request

5. **3D Reconstruction**: Combined scans form 3D point cloud



### Configuration

### 2. Launch Gazebo Simulation---

Adjust LiDAR rotation parameters in `lidar_rotation.launch.py`:



```python

parameters=[{Start the robot in a simulated environment:## License

    'rotation_speed': 0.5,    # rad/s

    'min_angle': -6.28,       # radians (~-360°)

    'max_angle': 6.25,        # radians (~360°)

}]```bashThis project is licensed under the terms specified in the LICENSE file.

```

ros2 launch reccobot_description gazebo.launch.py

### Visualization

```---

View 3D point cloud in RViz:



1. Add PointCloud2 display

2. Topic: `/scan` (transformed via TF)### 3. Start 3D Mapping## Contact

3. Fixed Frame: `base_link`

4. Decay Time: 10 seconds (to see accumulated points)



---In a new terminal, start the LiDAR rotation for 3D mapping:For questions or support, please open an issue on the GitHub repository.



## 🐛 Troubleshooting



### Issue: Package not found after building```bash**Repository:** [https://github.com/divyansh-ramola/RECCO_BOT](https://github.com/divyansh-ramola/RECCO_BOT)

source ~/reccobot_ws/install/setup.bash

**Solution:**ros2 launch reccobot_lidar_control lidar_rotation.launch.py

```bash```

source ~/reccobot_ws/install/setup.bash

```### 4. Control the Robot



### Issue: Gazebo fails to load robotUse keyboard teleop or the trained RL agent to move the robot.



**Solution:** Check Gazebo version and plugins:---

```bash

gz sim --version## 📦 Packages

ros2 pkg list | grep gazebo

```### reccobot_description



### Issue: LiDAR not publishing data**Complete robot description and simulation package**



**Solution:** Check device permissions:- URDF robot models with accurate kinematics

```bash- 3D mesh files for all robot components

sudo chmod 666 /dev/ttyUSB0  # or your LiDAR device- Gazebo simulation launch files

```- RViz visualization configurations

- Deep RL training scripts

### Issue: Controllers not loading

**Key Features:**

**Solution:** Verify controller configuration:- 12 DOF quadruped with 4 legs

```bash- Rotating LiDAR mount (1 DOF)

ros2 control list_hardware_interfaces- Camera pan-tilt mechanism (2 DOF)

ros2 control list_controllers- IMU sensor integration

```- Physics-accurate collision meshes



### Issue: Training crashes with CUDA errors📖 [Detailed Documentation](reccobot_description/README.md)



**Solution:** Use CPU for training:### reccobot_controllers

```bash

export CUDA_VISIBLE_DEVICES=-1**ROS 2 Control integration package**

python3 train_walking.py

```- Hardware-agnostic control interfaces

- Position and velocity controllers

### Issue: TF frames not updating- Joint trajectory controllers

- Controller configuration files

**Solution:** Check robot_state_publisher:

```bash**Supported Controllers:**

ros2 node list- Joint State Broadcaster

ros2 run tf2_tools view_frames- Position Controllers (per joint)

```- Forward Command Controller

- Joint Trajectory Controller

---

### reccobot_lidar_control

## 🤝 Contributing

**3D mapping through LiDAR rotation**

We welcome contributions! Please follow these steps:

- Continuous rotation of 2D LiDAR sensor

1. **Fork the repository**- Configurable rotation speed and range

2. **Create a feature branch**: `git checkout -b feature/amazing-feature`- Creates 3D point clouds from 2D scans

3. **Make your changes**: Implement your feature or bug fix- Real-time TF frame updates

4. **Test thoroughly**: Ensure all packages build and run

5. **Commit with clear messages**: `git commit -m 'Add amazing feature'`**Features:**

6. **Push to your fork**: `git push origin feature/amazing-feature`- Rotation range: -360° to +360°

7. **Open a Pull Request**: Describe your changes in detail- Adjustable speed: 0.1 to 2.0 rad/s

- Automatic direction reversal at limits

### Coding Standards- Synchronized with TF transforms



- Follow ROS 2 naming conventions📖 [Detailed Documentation](reccobot_lidar_control/README.md)

- Add comments and docstrings

- Update documentation for new features### reccobot_state_machine

- Test in both simulation and visualization

**High-level behavior coordination**

---

- State machine for robot behaviors

## 📄 License- Task planning and execution

- Sensor-driven state transitions

This project is licensed under the BSD-3-Clause License - see the LICENSE file for details.

### ldrobot-lidar-ros2

---

**LDRobot LiDAR driver integration**

## 📧 Contact

- ROS 2 node for LDRobot LD06/LD19 LiDAR

**Project Maintainer**: Divyansh Ramola- Publishes LaserScan messages

- SLAM integration support

**Email**: 2022ume1557@mnit.ac.in- Component-based architecture



**Repository**: [https://github.com/divyansh-ramola/RECCO_BOT](https://github.com/divyansh-ramola/RECCO_BOT)### Deep RL Framework (SpiderBot_DeepRL)



For questions, issues, or feature requests, please open an issue on GitHub.**Reinforcement learning for locomotion**



---- PPO (Proximal Policy Optimization) implementation

- Custom Gymnasium environment

## 🙏 Acknowledgments- Training and evaluation scripts

- Model checkpointing and logging

- **ROS 2 Community**: For the incredible robotics framework

- **Gazebo Team**: For the physics simulation environment---

- **Stable Baselines3**: For the RL implementations

- **LDRobot**: For the affordable LiDAR sensors## 🎯 Features in Detail



---### 3D Mapping System



<div align="center">The robot uses a rotating 2D LiDAR to create 3D point clouds:



**Built with ❤️ using ROS 2 and Python**1. **Hardware Setup**: 2D LiDAR mounted on a rotating servo

2. **Rotation Control**: Continuous back-and-forth rotation (-360° to +360°)

⭐ Star this repository if you find it helpful!3. **Data Collection**: 2D scans captured at different angles

4. **3D Reconstruction**: TF transforms combine scans into 3D space

</div>5. **Visualization**: Real-time point cloud in RViz


**Demo:**
```bash
# Terminal 1: Launch Gazebo
ros2 launch reccobot_description gazebo.launch.py

# Terminal 2: Start LiDAR rotation
ros2 launch reccobot_lidar_control lidar_rotation.launch.py

# Terminal 3: Visualize in RViz
ros2 launch reccobot_description display.launch.py
```

### Deep Reinforcement Learning

Train the quadruped to walk using PPO:

**Architecture:**
- **State Space**: Joint positions, velocities, body orientation, target velocity
- **Action Space**: Joint position commands (12 dimensions)
- **Reward Function**: Forward velocity, stability, energy efficiency
- **Algorithm**: PPO with MLP policy

**Training Process:**
```bash
# Start Gazebo simulation
ros2 launch reccobot_description gazebo.launch.py

# In another terminal, start training
cd ~/reccobot_ws/src/reccobot_description/scripts
python3 train_walking.py
```

**Monitor Training:**
```bash
tensorboard --logdir logs/
```

### ROS 2 Control Integration

Hardware-agnostic control using ros2_control:

- **Joint State Interface**: Reads joint positions and velocities
- **Position Command Interface**: Sends position commands to joints
- **Controller Manager**: Manages multiple controllers
- **Gazebo Integration**: Simulated hardware interface

**Launch Controllers:**
```bash
ros2 launch reccobot_controllers controller.launch.py
```

**List Active Controllers:**
```bash
ros2 control list_controllers
```

---

## 🎓 Training Deep RL Agent

### Training Script

The training script (`train_walking.py`) provides a complete RL training pipeline:

**Features:**
- Gymnasium-compatible environment
- PPO algorithm from stable-baselines3
- Automatic checkpointing
- Tensorboard logging
- Evaluation callbacks

### Custom Environment

The `ReccobotEnv` environment provides:

```python
class ReccobotEnv(gymnasium.Env):
    observation_space: Box(shape=(39,))  # Joint states + body pose + target
    action_space: Box(shape=(12,))       # Joint position commands
    
    reward = (
        + forward_velocity_reward
        - energy_consumption
        - deviation_from_upright
        + stability_bonus
    )
```

### Training Parameters

Modify training hyperparameters in `train_walking.py`:

```python
model = PPO(
    "MlpPolicy",
    env,
    learning_rate=3e-4,      # Learning rate
    n_steps=2048,            # Steps per update
    batch_size=64,           # Minibatch size
    n_epochs=10,             # Optimization epochs
    gamma=0.99,              # Discount factor
    clip_range=0.2,          # PPO clip range
)
```

### Evaluation

Test a trained model:

```python
python3 test_trained_model.py --model models/reccobot_walking_final.zip --episodes 10
```

---

## 🗺️ 3D Mapping with LiDAR

### How It Works

1. **2D LiDAR Scan**: LD06 provides 360° 2D scan at 10Hz
2. **Rotation Mechanism**: Servo rotates LiDAR through vertical axis
3. **Frame Synchronization**: TF tree maintains lidar pose at each scan
4. **Point Cloud Assembly**: Scans transformed to fixed frame and accumulated
5. **3D Reconstruction**: Combined scans form 3D point cloud

### Configuration

Adjust LiDAR rotation parameters in `lidar_rotation.launch.py`:

```python
parameters=[{
    'rotation_speed': 0.5,    # rad/s
    'min_angle': -6.28,       # radians (~-360°)
    'max_angle': 6.25,        # radians (~360°)
}]
```

### Visualization

View 3D point cloud in RViz:

1. Add PointCloud2 display
2. Topic: `/scan` (transformed via TF)
3. Fixed Frame: `base_link`
4. Decay Time: 10 seconds (to see accumulated points)

---

## 🐛 Troubleshooting

### Issue: Package not found after building

**Solution:**
```bash
source ~/reccobot_ws/install/setup.bash
```

### Issue: Gazebo fails to load robot

**Solution:** Check Gazebo version and plugins:
```bash
gz sim --version
ros2 pkg list | grep gazebo
```

### Issue: LiDAR not publishing data

**Solution:** Check device permissions:
```bash
sudo chmod 666 /dev/ttyUSB0  # or your LiDAR device
```

### Issue: Controllers not loading

**Solution:** Verify controller configuration:
```bash
ros2 control list_hardware_interfaces
ros2 control list_controllers
```

### Issue: Training crashes with CUDA errors

**Solution:** Use CPU for training:
```bash
export CUDA_VISIBLE_DEVICES=-1
python3 train_walking.py
```

### Issue: TF frames not updating

**Solution:** Check robot_state_publisher:
```bash
ros2 node list
ros2 run tf2_tools view_frames
```

---

## 🤝 Contributing

We welcome contributions! Please follow these steps:

1. **Fork the repository**
2. **Create a feature branch**: `git checkout -b feature/amazing-feature`
3. **Make your changes**: Implement your feature or bug fix
4. **Test thoroughly**: Ensure all packages build and run
5. **Commit with clear messages**: `git commit -m 'Add amazing feature'`
6. **Push to your fork**: `git push origin feature/amazing-feature`
7. **Open a Pull Request**: Describe your changes in detail

### Coding Standards

- Follow ROS 2 naming conventions
- Add comments and docstrings
- Update documentation for new features
- Test in both simulation and visualization

---

## 📄 License

This project is licensed under the BSD-3-Clause License - see the LICENSE file for details.

---

## 📧 Contact

**Project Maintainer**: Divyansh Ramola

**Email**: 2022ume1557@mnit.ac.in

**Repository**: [https://github.com/divyansh-ramola/RECCO_BOT](https://github.com/divyansh-ramola/RECCO_BOT)

For questions, issues, or feature requests, please open an issue on GitHub.

---

## 🙏 Acknowledgments

- **ROS 2 Community**: For the incredible robotics framework
- **Gazebo Team**: For the physics simulation environment
- **Stable Baselines3**: For the RL implementations
- **LDRobot**: For the affordable LiDAR sensors

---

<div align="center">

**Built with ❤️ using ROS 2 and Python**

⭐ Star this repository if you find it helpful!

</div>
