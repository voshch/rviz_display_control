cmake_minimum_required(VERSION 3.8)
project(rviz_display_control_msgs)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rcl_interfaces REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/DisplaySpec.msg"
  "msg/DisplaySet.msg"
  DEPENDENCIES rcl_interfaces
)

ament_export_dependencies(rosidl_default_runtime)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  find_package(rcl_interfaces REQUIRED)

  ament_add_gtest(test_messages test/test_messages.cpp)
  rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
  target_link_libraries(test_messages
    "${cpp_typesupport_target}"
    ${rcl_interfaces_TARGETS})
endif()

ament_package()
