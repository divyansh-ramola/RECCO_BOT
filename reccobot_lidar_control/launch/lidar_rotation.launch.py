from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='reccobot_lidar_control',
            executable='lidar_rotation_node',
            name='lidar_rotation_node',
            output='screen',
            parameters=[{
                'rotation_speed': 0.5,    # radians per second (adjust for desired speed)
                'min_angle': -6.28,       # minimum rotation angle in radians
                'max_angle': 6.25,        # maximum rotation angle in radians
            }]
        ),
    ])
