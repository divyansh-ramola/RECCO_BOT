import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, RegisterEventHandler
from launch.conditions import IfCondition
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    # Declare arguments
    declared_arguments = []
    declared_arguments.append(
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'))
    
    declared_arguments.append(
        DeclareLaunchArgument(
            'controller_config_file',
            default_value='lidar_controller.yaml',
            description='Controller configuration file'))

    # Initialize Arguments
    use_sim_time = LaunchConfiguration('use_sim_time')
    controller_config_file = LaunchConfiguration('controller_config_file')

    # Get URDF via xacro
    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name='xacro')]),
            ' ',
            PathJoinSubstitution([
                FindPackageShare('reccobot_description'),
                'urdf',
                'reccobot.urdf.xacro'
            ]),
        ]
    )
    robot_description = {'robot_description': robot_description_content}

    # Controller configuration
    robot_controllers = PathJoinSubstitution([
        FindPackageShare('reccobot_lidar_control'),
        'config',
        controller_config_file,
    ])

    # Control node
    control_node = Node(
        package='controller_manager',
        executable='ros2_control_node',
        parameters=[robot_description, robot_controllers],
        output='both',
        remappings=[
            ('/controller_manager/robot_description', '/robot_description'),
        ],
    )

    # Robot state publisher
    robot_state_pub_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='both',
        parameters=[robot_description, {'use_sim_time': use_sim_time}],
    )

    # Spawn controller
    lidar_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['lidar_position_controller', '--controller-manager', '/controller_manager'],
        output='screen',
    )

    # Delay controller spawner after control_node
    delay_lidar_controller_spawner = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=control_node,
            on_exit=[lidar_controller_spawner],
        )
    )

    nodes = [
        control_node,
        robot_state_pub_node,
        lidar_controller_spawner,
    ]

    return LaunchDescription(declared_arguments + nodes)
