import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def launch_setup(context, *args, **kwargs):
    # Get launch arguments
    serial_port = LaunchConfiguration('serial_port').perform(context)
    baud_rate = LaunchConfiguration('baud_rate').perform(context)
    controller_config_file = LaunchConfiguration('controller_config_file').perform(context)

    # Minimal robot description for just the LiDAR joint
    robot_description_content = f"""<?xml version="1.0"?>
<robot name="lidar_rotation">
  <link name="base_link"/>
  
  <joint name="base_coupling" type="continuous">
    <parent link="base_link"/>
    <child link="lidar_link"/>
    <origin xyz="0 0 0.1" rpy="0 0 0"/>
    <axis xyz="0 0 1"/>
  </joint>
  
  <link name="lidar_link">
    <visual>
      <geometry>
        <cylinder radius="0.05" length="0.1"/>
      </geometry>
    </visual>
  </link>
  
  <ros2_control name="ArduinoLidarSystem" type="system">
    <hardware>
      <plugin>reccobot_lidar_control/ArduinoLidarHardware</plugin>
      <param name="serial_port">{serial_port}</param>
      <param name="baud_rate">{baud_rate}</param>
    </hardware>
    <joint name="base_coupling">
      <command_interface name="position"/>
      <state_interface name="position"/>
      <state_interface name="velocity"/>
    </joint>
  </ros2_control>
</robot>
"""
    
    robot_description = {'robot_description': robot_description_content}

    # Get package share directory
    pkg_share = FindPackageShare('reccobot_lidar_control').find('reccobot_lidar_control')
    
    # Controller configuration
    robot_controllers = os.path.join(pkg_share, 'config', controller_config_file)

    # Control node
    control_node = Node(
        package='controller_manager',
        executable='ros2_control_node',
        parameters=[robot_description, robot_controllers],
        output='both',
    )

    # Robot state publisher
    robot_state_pub_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='both',
        parameters=[robot_description],
    )

    # Spawn controller
    lidar_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['lidar_position_controller', '--controller-manager', '/controller_manager'],
        output='screen',
    )

    return [
        control_node,
        robot_state_pub_node,
        lidar_controller_spawner,
    ]


def generate_launch_description():
    # Declare arguments
    declared_arguments = []
    declared_arguments.append(
        DeclareLaunchArgument(
            'controller_config_file',
            default_value='lidar_controller.yaml',
            description='Controller configuration file'))
    
    declared_arguments.append(
        DeclareLaunchArgument(
            'serial_port',
            default_value='/dev/ttyUSB0',
            description='Arduino serial port'))
    
    declared_arguments.append(
        DeclareLaunchArgument(
            'baud_rate',
            default_value='115200',
            description='Serial baud rate'))

    return LaunchDescription(declared_arguments + [OpaqueFunction(function=launch_setup)])
