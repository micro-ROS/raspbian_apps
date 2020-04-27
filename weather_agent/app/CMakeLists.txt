cmake_minimum_required(VERSION 3.5)

project(weather_agent LANGUAGES CXX)

find_package(ament_cmake REQUIRED)
find_package(microxrcedds_agent REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
  microxrcedds_agent
  )

install(
  TARGETS
    ${PROJECT_NAME}
  DESTINATION
    ${PROJECT_NAME}
  )