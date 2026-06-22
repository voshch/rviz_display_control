cmake_minimum_required(VERSION 3.8)
project(rviz_display_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rviz_common REQUIRED)
find_package(rviz_display_control_msgs REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(Qt5 QUIET COMPONENTS Widgets)
if(Qt5_FOUND)
  set(qt_widgets Qt5::Widgets)
else()
  find_package(Qt6 REQUIRED COMPONENTS Widgets)
  set(qt_widgets Qt6::Widgets)
endif()

set(CMAKE_AUTOMOC ON)

add_library(reconciler
  src/reconciler.cpp
  src/reconcile_core.cpp
  include/rviz_display_control/reconciler.hpp)
target_include_directories(reconciler PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(reconciler
  rclcpp::rclcpp
  pluginlib::pluginlib
  rviz_common::rviz_common
  ${rviz_display_control_msgs_TARGETS}
  ${rcl_interfaces_TARGETS}
)
target_link_libraries(reconciler ${qt_widgets})

# Surface missing symbols (e.g. un-generated moc output) at link time, not plugin load.
target_link_options(reconciler PRIVATE "LINKER:--no-undefined")

install(TARGETS reconciler
        EXPORT export_rviz_display_control
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)
install(DIRECTORY include/
        DESTINATION include
)
install(FILES plugin.xml
        DESTINATION share/${PROJECT_NAME}
)
ament_export_include_directories(include)
ament_export_targets(export_rviz_display_control)
pluginlib_export_plugin_description_file(rviz_common plugin.xml)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_reconcile_core
    test/test_reconcile_core.cpp
    src/reconcile_core.cpp)
  target_include_directories(test_reconcile_core PRIVATE include)
  target_link_libraries(test_reconcile_core
    ${rviz_display_control_msgs_TARGETS}
    ${rcl_interfaces_TARGETS})
endif()

ament_package()
