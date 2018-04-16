cmake_minimum_required(VERSION 3.5)

project(rmw_ndn)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_ros REQUIRED)

find_package(rmw REQUIRED)
find_package(rosidl_typesupport_cbor REQUIRED)

include_directories(include)

add_library(rmw_ndn
  src/app.cpp
  src/app.h
  src/functions.c
  src/rmw_guard.cpp
  src/rmw_node.cpp
  src/rmw_pub.cpp
  src/rmw_sub.cpp
  src/rmw_wait.cpp
)

INCLUDE (FindPkgConfig)
pkg_check_modules (NDN_CXX libndn-cxx)

include_directories(rmw_ndn ${NDN_CXX_INCLUDE_DIRS})
link_directories(rmw_ndn ${NDN_CXX_LIBRARY_DIRS})
target_link_libraries(rmw_ndn ${NDN_CXX_LIBRARIES})
add_compile_options(-fPIC)

# specific order: dependents before dependencies
ament_target_dependencies(rmw_ndn
  "rosidl_typesupport_cbor"
  "rmw"
)

configure_rmw_library(rmw_ndn)

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME}
PRIVATE "RMW_NDN_BUILDING_LIBRARY")

# specific order: dependents before dependencies
ament_export_include_directories(include)
ament_export_libraries(rmw_ndn)

ament_export_dependencies(rosidl_typesupport_cbor)
ament_export_dependencies(rmw)

register_rmw_implementation(
  "c:rosidl_typesupport_c:rosidl_typesupport_cbor"
  "cpp:rosidl_typesupport_cpp:rosidl_typesupport_cbor")

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

install(
  TARGETS rmw_ndn
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
