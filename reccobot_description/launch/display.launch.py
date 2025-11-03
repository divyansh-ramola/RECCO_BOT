import os
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
from launch.actions import DeclareLaunchArgument
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command


def generate_launch_description():
    pkg_share = get_package_share_directory('reccobot_description')
    urdf_default = os.path.join(pkg_share, 'urdf', 'reccobot_description.urdf')
    xacro_file = LaunchConfiguration('xacro_file')
    initial_positions = LaunchConfiguration('initial_positions')
    rviz_config_path = os.path.join(pkg_share, 'urdf.rviz')

    # Declare file arguments (xacro/urdf and initial joint positions YAML)
    xacro_file_arg = DeclareLaunchArgument(
        'xacro_file',
        default_value=PathJoinSubstitution([pkg_share, 'urdf', 'reccobot_description.urdf']),
        description='Path to the Xacro/URDF to load (will be expanded via xacro)'
    )

    initial_positions_arg = DeclareLaunchArgument(
        'initial_positions',
        default_value=PathJoinSubstitution([
            pkg_share, 'config', 'initial_joint_positions.yaml'
        ]),
        description='YAML file with initial joint positions for joint_state_publisher'
    )

    # Robot State Publisher - expand xacro/urdf and publish TF
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': ParameterValue(Command(['xacro ', xacro_file]), value_type=str),
            'publish_frequency': 30.0  # Hz
        }]
    )

    # Joint State Publisher GUI - allows manual control of joints
    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        output='screen',
        parameters=[initial_positions]
    )

    # RViz2 - visualization with auto TF display
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_path] if os.path.exists(rviz_config_path) else [],
        output='screen',
        parameters=[{
            'use_sim_time': False
        }]
    )

    ld = LaunchDescription()
    ld.add_action(xacro_file_arg)
    ld.add_action(initial_positions_arg)
    
    # Add nodes in order
    ld.add_action(robot_state_publisher_node)
    ld.add_action(joint_state_publisher_gui_node)
    ld.add_action(rviz_node)

    return ld
