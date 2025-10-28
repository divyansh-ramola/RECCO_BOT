
cmake_minimum_required(VERSION 3.5)

project(urdf_tutorial)

find_package(ament_cmake REQUIRED)

# Install package resources
install(
  DIRECTORY images meshes launch rviz urdf
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
