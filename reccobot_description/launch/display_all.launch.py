#!/usr/bin/env python3
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command


def generate_launch_description():
    pkg_share = FindPackageShare('reccobot_description')

    xacro_file = LaunchConfiguration('xacro_file')
    initial_positions = LaunchConfiguration('initial_positions')
    rviz_config = LaunchConfiguration('rviz_config')

    return LaunchDescription([
        DeclareLaunchArgument(
            'xacro_file',
            default_value=PathJoinSubstitution([
                pkg_share, 'urdf', 'reccobot_description.urdf'
            ]),
            description='Path to the Xacro/URDF file to load'
        ),
        DeclareLaunchArgument(
            'initial_positions',
            default_value=PathJoinSubstitution([
                pkg_share, 'config', 'initial_joint_positions.yaml'
            ]),
            description='YAML file with initial joint positions for joint_state_publisher'
        ),
        DeclareLaunchArgument(
            'rviz_config',
            default_value=PathJoinSubstitution([
                pkg_share, 'urdf.rviz'
            ]),
            description='Path to RViz configuration file'
        ),

        # Robot State Publisher: expands Xacro/URDF and publishes TF
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{
                'robot_description': ParameterValue(Command(['xacro ', xacro_file]), value_type=str)
            }]
        ),

        # Joint State Publisher GUI: manual joint control with initial zeros
        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            name='joint_state_publisher_gui',
            output='screen',
            parameters=[initial_positions]
        ),

        # RViz2: visualization
        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            arguments=['-d', rviz_config]
        ),
    ])
