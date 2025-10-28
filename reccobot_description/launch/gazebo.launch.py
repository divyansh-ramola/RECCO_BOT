import os

from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, ExecuteProcess
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_share = get_package_share_directory('reccobot_description')
    urdf_path = os.path.join(pkg_share, 'urdf', 'reccobot_description.urdf')

    # Include gazebo_ros default launch (empty world)
    gazebo_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            os.path.join(get_package_share_directory('gazebo_ros'), 'launch', 'gazebo.launch.py')
        ])
    )

    # static transform publisher (frame: base_link -> base_footprint)
    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='tf_footprint_base',
        arguments=['0', '0', '0', '0', '0', '0', 'base_link', 'base_footprint', '40'],
        output='screen'
    )

    # spawn the robot into gazebo
    spawn_model = Node(
        package='gazebo_ros',
        executable='spawn_entity.py',
        arguments=['-file', urdf_path, '-entity', 'reccobot_description'],
        output='screen'
    )

    # publish calibrated=true once (or repeatedly) — mimic original behaviour
    publish_calibrated = ExecuteProcess(
        cmd=['ros2', 'topic', 'pub', '--once', '/calibrated', 'std_msgs/msg/Bool', "{data: true}"],
        output='screen'
    )

    ld = LaunchDescription()
    ld.add_action(gazebo_launch)
    ld.add_action(static_tf)
    ld.add_action(spawn_model)
    ld.add_action(publish_calibrated)

    return ld
