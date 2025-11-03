#!/usr/bin/env python3
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch.conditions import IfCondition, UnlessCondition
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command


def generate_launch_description():
    pkg_share = FindPackageShare('reccobot_description')

    xacro_file = LaunchConfiguration('xacro_file')
    initial_positions = LaunchConfiguration('initial_positions')
    use_gui = LaunchConfiguration('use_gui')

    return LaunchDescription([
        DeclareLaunchArgument(
            'xacro_file',
            default_value=PathJoinSubstitution([
                pkg_share, 'urdf', 'reccobot_description.urdf'
            ]),
            description='Path to the Xacro file to load'
        ),
        DeclareLaunchArgument(
            'initial_positions',
            default_value=PathJoinSubstitution([
                pkg_share, 'config', 'initial_joint_positions.yaml'
            ]),
            description='YAML file with initial joint positions for joint_state_publisher'
        ),
        DeclareLaunchArgument(
            'use_gui',
            default_value='true',
            description='Whether to start joint_state_publisher_gui instead of joint_state_publisher'
        ),

        # Robot State Publisher: expands Xacro and publishes TF
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{
                'robot_description': ParameterValue(Command(['xacro ', xacro_file]), value_type=str)
            }]
        ),

        # Joint State Publisher (switchable GUI)
        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            name='joint_state_publisher_gui',
            output='screen',
            condition=IfCondition(use_gui),
            parameters=[initial_positions]
        ),
        Node(
            package='joint_state_publisher',
            executable='joint_state_publisher',
            name='joint_state_publisher',
            output='screen',
            condition=UnlessCondition(use_gui),
            parameters=[initial_positions]
        ),
    ])
