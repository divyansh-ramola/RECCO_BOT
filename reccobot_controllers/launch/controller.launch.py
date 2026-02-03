import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():

    # Use this flag to switch between simulation and real hardware
    # - is_sim:=true  -> full sim stack (Gazebo + sim controllers; this launch mostly unused)
    # - is_sim:=false -> real robot: robot_state_publisher + ros2_control + hardware lidar driver
    is_sim_arg = DeclareLaunchArgument(
        "is_sim",
        default_value="false",
        description="If true, assume simulation; if false, launch real hardware controllers and Arduino lidar driver."
    )

    is_sim = LaunchConfiguration("is_sim")

    robot_description = ParameterValue(
        Command(
            [
                "xacro ",
                os.path.join(
                    get_package_share_directory("reccobot_description"),
                    "urdf",
                    "reccobot_description.urdf",
                ),
                " sim_mode:=",
                is_sim,
            ]
        ),
        value_type=str,
    )

    # On real hardware, publish robot_description and TF from the URDF
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        condition=UnlessCondition(is_sim),
        parameters=[{"robot_description": robot_description}],
    )

    # ros2_control for real hardware (not used in simulation)
    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            {"robot_description": robot_description,
             "use_sim_time": is_sim},
            os.path.join(
                get_package_share_directory("reccobot_description"),
                "config",
                "reccobot_controllers.yaml",
            ),
        ],
        condition=UnlessCondition(is_sim),
    )

    # Controller spawners are only meaningful when ros2_control_node runs
    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager",
        ],
        condition=UnlessCondition(is_sim),
    )

    leg_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["leg_controller", "--controller-manager", "/controller_manager"],
        condition=UnlessCondition(is_sim),
    )

    camera_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["camera_controller", "--controller-manager", "/controller_manager"],
        condition=UnlessCondition(is_sim),
    )

    lidar_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["lidar_controller", "--controller-manager", "/controller_manager"],
        condition=UnlessCondition(is_sim),
    )

    # Arduino lidar hardware is now handled by ArduinoLidarHardware plugin via ros2_control
    # No separate driver node needed anymore - the plugin communicates with Arduino directly

    return LaunchDescription(
        [
            is_sim_arg,
            robot_state_publisher_node,
            controller_manager,
            joint_state_broadcaster_spawner,
            leg_controller_spawner,
            camera_controller_spawner,
            lidar_controller_spawner,
        ]
    )
