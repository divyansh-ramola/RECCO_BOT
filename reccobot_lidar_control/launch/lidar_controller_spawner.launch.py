import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


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
            'continuous_scan',
            default_value='true',
            description='Enable continuous scanning mode'))
    
    declared_arguments.append(
        DeclareLaunchArgument(
            'scan_speed',
            default_value='1.0',
            description='Scan speed in rad/s'))

    # Initialize Arguments
    controller_config_file = LaunchConfiguration('controller_config_file')
    continuous_scan = LaunchConfiguration('continuous_scan')
    scan_speed = LaunchConfiguration('scan_speed')

    # Controller configuration
    robot_controllers = PathJoinSubstitution([
        FindPackageShare('reccobot_lidar_control'),
        'config',
        controller_config_file,
    ])

    # Spawn controller
    lidar_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'lidar_position_controller',
            '--controller-manager', '/controller_manager',
            '--param-file', robot_controllers,
        ],
        output='screen',
        parameters=[{
            'continuous_scan': continuous_scan,
            'scan_speed': scan_speed,
        }]
    )

    nodes = [
        lidar_controller_spawner,
    ]

    return LaunchDescription(declared_arguments + nodes)
