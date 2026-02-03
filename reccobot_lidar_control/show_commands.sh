#!/bin/bash

cat << 'EOF'

═══════════════════════════════════════════════════════════════════════════════
                    🎉 LiDAR Control System Ready!
═══════════════════════════════════════════════════════════════════════════════

✅ Problem Fixed: Created standalone launch file that doesn't require full robot URDF

═══════════════════════════════════════════════════════════════════════════════
                         📋 USAGE INSTRUCTIONS
═══════════════════════════════════════════════════════════════════════════════

STEP 1: Test Arduino Connection (RECOMMENDED FIRST!)
-------------------------------------------------------
ros2 run reccobot_lidar_control test_arduino_connection.py

# Or for different port:
ros2 run reccobot_lidar_control test_arduino_connection.py /dev/ttyACM0


STEP 2: Launch the Control System
-------------------------------------------------------
ros2 launch reccobot_lidar_control lidar_control_standalone.launch.py

# Or specify serial port:
ros2 launch reccobot_lidar_control lidar_control_standalone.launch.py serial_port:=/dev/ttyACM0


STEP 3: Control the LiDAR (in new terminal)
-------------------------------------------------------
# Move to 90 degrees
ros2 topic pub /lidar_position_controller/command std_msgs/msg/Float64 "data: 1.5708" --once

# Monitor position
ros2 topic echo /lidar_position_controller/state

# Run examples
ros2 run reccobot_lidar_control lidar_control_example.py


STEP 4: Check Status
-------------------------------------------------------
ros2 control list_controllers
ros2 control list_hardware_interfaces

═══════════════════════════════════════════════════════════════════════════════

📖 Full documentation: src/reccobot_lidar_control/LAUNCH_GUIDE.md

═══════════════════════════════════════════════════════════════════════════════

EOF
