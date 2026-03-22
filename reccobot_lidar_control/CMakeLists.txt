cmake_minimum_required(VERSION 3.8)
project(reccobot_lidar_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(controller_interface REQUIRED)
find_package(realtime_tools REQUIRED)

add_executable(lidar_rotation_node src/lidar_rotation_node.cpp)
add_executable(lidar_to_cloud src/lidar_to_cloud.cpp)
target_include_directories(lidar_rotation_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(lidar_rotation_node PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  lidar_rotation_node
  "rclcpp"
  "std_msgs"
  "sensor_msgs"
)
ament_target_dependencies(lidar_to_cloud
  "rclcpp"
  "sensor_msgs"
  "tf2_ros"
  "tf2"
  "tf2_geometry_msgs"
)

# Build Arduino hardware interface plugin
add_library(arduino_lidar_hardware SHARED
  src/arduino_lidar_hardware.cpp
)
target_include_directories(arduino_lidar_hardware PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(arduino_lidar_hardware PUBLIC cxx_std_17)
ament_target_dependencies(arduino_lidar_hardware
  "hardware_interface"
  "pluginlib"
  "rclcpp"
  "rclcpp_lifecycle")

# Export hardware plugin
pluginlib_export_plugin_description_file(hardware_interface arduino_lidar_hardware_plugin.xml)

# Build LiDAR Position Controller
add_library(lidar_position_controller SHARED
  src/lidar_position_controller.cpp
)
target_include_directories(lidar_position_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(lidar_position_controller PUBLIC cxx_std_17)
ament_target_dependencies(lidar_position_controller
  "controller_interface"
  "pluginlib"
  "rclcpp"
  "rclcpp_lifecycle"
  "realtime_tools"
  "std_msgs")

# Export controller plugin
pluginlib_export_plugin_description_file(controller_interface lidar_controller_plugin.xml)

install(TARGETS lidar_rotation_node lidar_to_cloud
  DESTINATION lib/${PROJECT_NAME})

install(TARGETS arduino_lidar_hardware lidar_position_controller
  DESTINATION lib)

# Install launch files
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/)

# Install config files
install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}/)

# Install arduino sketches
install(DIRECTORY arduino
  DESTINATION share/${PROJECT_NAME}/)

# Install Python scripts
install(PROGRAMS
  scripts/lidar_control_example.py
  scripts/test_arduino_connection.py
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
