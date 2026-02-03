from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    sim = LaunchConfiguration('sim')

    declare_sim = DeclareLaunchArgument(
        'sim',
        default_value='true',
        description='If true, use Gazebo/sim controllers; if false, use Arduino hardware driver.'
    )

    rotation_speed = 4.0

    sim_node = Node(
        package='reccobot_lidar_control',
        executable='lidar_rotation_node',
        name='lidar_rotation_node',
        output='screen',
        parameters=[{
            'rotation_speed': rotation_speed,
            'min_angle': -6.28,
            'max_angle': 6.25,
        }],
        condition=IfCondition(sim)
    )

    hw_node = Node(
        package='reccobot_lidar_control',
        executable='arduino_lidar_driver_node',
        name='arduino_lidar_driver',
        output='screen',
        parameters=[{
            'ip': '192.168.1.150',
            'state_port': 8000,
            'command_port': 8001,
            'joint_name': 'base_coupling',
        }],
        condition=UnlessCondition(sim)
    )

    # 3D cloud accumulation node, always started but frame params differ between sim and HW.
    # For real hardware (ldlidar_node), we want lidar_frame:=ldlidar_link, fixed_frame:=base_link
    # In pure Gazebo sim, you can override these via launch arguments if needed.
    cloud_node = Node(
        package='reccobot_lidar_control',
        executable='lidar_to_cloud',
        name='lidar_to_cloud',
        output='screen',
        parameters=[{
            'lidar_frame': 'ldlidar_link',
            'fixed_frame': 'base_link',
        }]
    )

    return LaunchDescription([
        declare_sim,
        sim_node,
        hw_node,
        cloud_node,
    ])

