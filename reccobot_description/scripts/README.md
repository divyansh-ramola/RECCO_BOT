# RECCOBOT scripts

Quick helpers for testing and demonstrating the quadruped in ROS 2.

Base frame note: The physical body frame is `chassis_link` (with `base_link` as an empty root connected via a fixed joint). Scripts here don’t depend on TF frames; they publish joint commands directly.

Topics used:
- /leg_controller/commands (Float64MultiArray, 12 values: 4 legs × [coxa, femur, tibia])
- /camera_controller/commands (Float64MultiArray, 2 values)
- /lidar_controller/commands (Float64MultiArray, 1 value)
- /joint_states (sensor_msgs/JointState)


Tips:
- Launch Gazebo first: ros2 launch reccobot_description gazebo.launch.py
- If the robot doesn’t move, ensure controllers are active: ros2 control list_controllers
- Standing pose reference (per leg): [0.0, -0.3, 0.6]
