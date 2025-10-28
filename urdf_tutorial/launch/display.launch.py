import os
import shlex
import subprocess

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def _prepare(context, *args, **kwargs):
    model_path = LaunchConfiguration('model').perform(context)

    # If given a xacro file, run xacro to generate the URDF
    robot_description = ''
    try:
        if model_path.endswith('.xacro'):
            cmd = ['xacro', model_path]
            robot_description = subprocess.check_output(cmd, universal_newlines=True)
        else:
            with open(model_path, 'r') as f:
                robot_description = f.read()
    except Exception:
        robot_description = ''

    # create nodes
    joint_state_pub = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        name='joint_state_publisher_gui',
        output='screen'
    )

    robot_state_pub = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description}]
    )

    # default rviz config in package share if exists
    pkg_share = get_package_share_directory('urdf_tutorial')
    rviz_cfg = os.path.join(pkg_share, 'rviz', 'urdf.rviz')
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_cfg]
    )

    return [joint_state_pub, robot_state_pub, rviz_node]


def generate_launch_description():
    default_model = os.path.join(get_package_share_directory('urdf_tutorial'), 'urdf', '01-myfirst.urdf')

    ld = LaunchDescription()
    ld.add_action(DeclareLaunchArgument('model', default_value=default_model, description='Path to robot model (URDF or XACRO)'))
    ld.add_action(OpaqueFunction(function=_prepare))
    return ld
