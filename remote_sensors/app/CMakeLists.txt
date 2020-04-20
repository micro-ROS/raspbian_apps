cmake_minimum_required(VERSION 3.5)

project(remote_sensors LANGUAGES C)

add_executable(${PROJECT_NAME} app.c)

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD
    99
  C_STANDARD_REQUIRED
    YES
  )

install(
  TARGETS
    ${PROJECT_NAME}
  DESTINATION
    bin/${PROJECT_NAME}
  )