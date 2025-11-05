import os
from pathlib import Path
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.conditions import IfCondition, UnlessCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue

def generate_launch_description():
    pkg_reccobot_description = get_package_share_directory("reccobot_description")
    pkg_reccobot_controllers = get_package_share_directory("reccobot_controllers")

    # xacro to URDF
    model_arg = DeclareLaunchArgument(
        name="model",
        default_value=os.path.join(pkg_reccobot_description, "urdf", "reccobot_description.urdf"),
        description="Path to URDF"
    )

    # Choose simulator (Ignition/GZ vs Gazebo Classic)
    use_ignition_arg = DeclareLaunchArgument(
        name="use_ignition",
        default_value="true",
        description="Use Ignition Gazebo (ros_gz_sim). If false, use Gazebo Classic."
    )

    # Headless mode (server-only) to avoid EGL/GUI issues on headless systems
    headless_arg = DeclareLaunchArgument(
        name="headless",
        default_value="true",
        description="Run Ignition Gazebo server-only (no GUI) to avoid EGL/GUI issues"
    )

    # Optionally skip starting the simulator to attach to an already running GZ instance
    start_sim_arg = DeclareLaunchArgument(
        name="start_sim",
        default_value="true",
        description="Start Ignition/GZ sim within this launch. Set false to only spawn robot into an existing world."
    )

    # Spawn height to avoid initial ground collision/stiction
    spawn_z_arg = DeclareLaunchArgument(
        name="spawn_z",
        default_value="0.3",
        description="Spawn height above ground in meters"
    )

    # Entity name and deletion behavior to avoid duplicates
    entity_name_arg = DeclareLaunchArgument(
        name="entity_name",
        default_value="reccobot",
        description="Name of the spawned robot entity in the simulation"
    )
    delete_existing_arg = DeclareLaunchArgument(
        name="delete_existing",
        default_value="false",
        description="Delete any existing entity with the same name before spawning"
    )

    # Forward the sim selection into xacro (is_ignition arg)
    robot_description = ParameterValue(
        Command(["xacro ", LaunchConfiguration("model"), " is_ignition:=", LaunchConfiguration("use_ignition")]),
        value_type=str
    )

    # Set Gazebo (Ignition) resource paths for both legacy (Fortress: IGN_*) and newer (Garden+: GZ_*) env vars
    workspace_share_parent = str(Path(pkg_reccobot_description).parent.resolve())
    gazebo_resource_path_gz = SetEnvironmentVariable(
        name="GZ_SIM_RESOURCE_PATH",
        value=[workspace_share_parent]
    )
    gazebo_resource_path_ign = SetEnvironmentVariable(
        name="IGN_GAZEBO_RESOURCE_PATH",
        value=[workspace_share_parent]
    )

    # Launch Ignition Gazebo
    # Use server-only (-s) when headless to avoid EGL errors; otherwise start GUI
    gz_args = [
        '-s -r empty.sdf' if LaunchConfiguration('headless') == 'true' else '-r empty.sdf'
    ]
    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([
                get_package_share_directory('ros_gz_sim'),
                'launch',
                'gz_sim.launch.py'
            ])
        ]),
        launch_arguments={'gz_args': gz_args[0]}.items(),
        condition=IfCondition(
            PythonExpression([
                "'", LaunchConfiguration('use_ignition'), "' == 'true' and '",
                LaunchConfiguration('start_sim'), "' == 'true'"
            ])
        )
    )

    # Launch Gazebo Classic
    gz_classic = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            PathJoinSubstitution([
                get_package_share_directory('gazebo_ros'),
                'launch',
                'gazebo.launch.py'
            ])
        ]),
        condition=UnlessCondition(LaunchConfiguration('use_ignition'))
    )

    # Publish robot description to /robot_description
    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description, "use_sim_time": True}],
        output="screen"
    )

    # Spawn entity in Gazebo
    # Note: Node.arguments must be Substitutions/strings. Use Command(...) and not ParameterValue here.
    # Optional pre-remove step to ensure idempotent spawns
    remove_entity = Node(
        package="ros_gz_sim",
        executable="remove",
        arguments=[
            "-name", LaunchConfiguration('entity_name')
        ],
        output="screen",
        condition=IfCondition(
            PythonExpression([
                "'", LaunchConfiguration('use_ignition'), "' == 'true' and '",
                LaunchConfiguration('delete_existing'), "' == 'true'"
            ])
        )
    )

    spawn_entity = Node(
        package="ros_gz_sim",
        executable="create",
        arguments=[
            "-string", Command(["xacro ", LaunchConfiguration("model")]),
            "-name", LaunchConfiguration('entity_name'),
            "-x", "0.0",
            "-y", "0.0",
            "-z", LaunchConfiguration('spawn_z')
        ],
        output="screen",
        condition=IfCondition(LaunchConfiguration('use_ignition'))
    )

    # Spawn entity in Gazebo Classic
    spawn_entity_classic = Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        arguments=[
            "-topic", "robot_description",
            "-entity", LaunchConfiguration('entity_name'),
            "-x", "0.0",
            "-y", "0.0",
            "-z", LaunchConfiguration('spawn_z')
        ],
        output="screen",
        condition=UnlessCondition(LaunchConfiguration('use_ignition'))
    )

    # Bridge /clock
    clock_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        # Fortress uses ignition.msgs.* types; newer distros accept gz.msgs.*. Use ignition.* to match installed libs.
        arguments=["/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock]"],
        output="screen",
        condition=IfCondition(LaunchConfiguration('use_ignition'))
    )

    return LaunchDescription([
        model_arg,
        use_ignition_arg,
        headless_arg,
        start_sim_arg,
    spawn_z_arg,
        entity_name_arg,
        delete_existing_arg,
        gazebo_resource_path_gz,
        gazebo_resource_path_ign,
        gz_sim,
        gz_classic,
        robot_state_publisher,
        remove_entity,
        spawn_entity,
        spawn_entity_classic,
        clock_bridge
    ])
