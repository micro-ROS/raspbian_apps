cmake_minimum_required(VERSION 3.5)

project(weather_publisher LANGUAGES C)

find_package(ament_cmake REQUIRED)
find_package(microxrcedds_client REQUIRED)
find_package(rcl REQUIRED)
find_package(rmw_microxrcedds REQUIRED)
find_package(sensor_msgs REQUIRED)

add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME}
  rcl::rcl
  ${rmw_microxrcedds_LIBRARIES}
  ${sensor_msgs_TARGETS}
  )

target_link_libraries(${PROJECT_NAME}
  microxrcedds_client
  )

install(
  TARGETS
    ${PROJECT_NAME}
  DESTINATION
    ${PROJECT_NAME}
  )