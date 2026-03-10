cmake_minimum_required(VERSION 3.8)
project(docking_utils)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(rclcpp_components REQUIRED)

# 1. Build the Component Library
add_library(tf2_pose_node_component SHARED
  src/tf2_pose_node.cpp
)

target_include_directories(tf2_pose_node_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(tf2_pose_node_component
  rclcpp
  rclcpp_components
  geometry_msgs
  tf2_ros
  tf2_geometry_msgs
)

# Register the component
rclcpp_components_register_nodes(tf2_pose_node_component 
  "docking_utils::Tf2PoseNode"
)

# 2. Build the Standalone Executable
add_executable(tf2_pose_node src/tf2_pose_standalone.cpp)
target_link_libraries(tf2_pose_node tf2_pose_node_component)
ament_target_dependencies(tf2_pose_node rclcpp)

# 3. Install
install(TARGETS
  tf2_pose_node_component
  tf2_pose_node
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin/${PROJECT_NAME}
)






add_executable(tag_to_dock_bridge src/tag_to_dock_bridge.cpp)

target_include_directories(tag_to_dock_bridge PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

target_compile_features(tag_to_dock_bridge PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17

ament_target_dependencies(
  tag_to_dock_bridge
  "rclcpp"
  "geometry_msgs"
  "std_msgs"
  "tf2_ros"
  "tf2_geometry_msgs"
  "rclcpp_lifecycle"
)

add_library(tag_bridge_component SHARED src/lifecycle_tag_bridge_component.cpp)

target_include_directories(tag_bridge_component PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
  
ament_target_dependencies(tag_bridge_component
  rclcpp
  rclcpp_lifecycle
  rclcpp_components
  geometry_msgs
  tf2_ros
  tf2_geometry_msgs
)

install(TARGETS tag_to_dock_bridge
  DESTINATION lib/${PROJECT_NAME})


  
install(DIRECTORY config launch 
  DESTINATION share/${PROJECT_NAME}
)

# rclcpp_components_register_nodes(tag_bridge_component "docking_utils::LifecycleTagBridge")

rclcpp_components_register_node(tag_bridge_component
    PLUGIN "LifecycleTagBridge"
    EXECUTABLE "docking_utils"
)

target_compile_definitions(tag_bridge_component PRIVATE "COMPOSITION_BUILDING_DLL")

install(TARGETS tag_bridge_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

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
