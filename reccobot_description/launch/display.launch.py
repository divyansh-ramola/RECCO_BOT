import os

from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_share = get_package_share_directory('reccobot_description')
    urdf_path = os.path.join(pkg_share, 'urdf', 'reccobot_description.urdf')
    rviz_config_path = os.path.join(pkg_share, 'urdf.rviz')

    # Read URDF file
    robot_description = ''
    try:
        with open(urdf_path, 'r') as inf:
            robot_description = inf.read()
    except Exception as e:
        print(f"Error reading URDF file: {e}")
        robot_description = ''

    # Robot State Publisher - publishes TF transforms based on URDF
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description,
            'publish_frequency': 30.0  # Hz
        }]
    )

    # Joint State Publisher GUI - allows manual control of joints
    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        output='screen'
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
    
    # Add nodes in order
    ld.add_action(robot_state_publisher_node)
    ld.add_action(joint_state_publisher_gui_node)
    ld.add_action(rviz_node)

    return ld
