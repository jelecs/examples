cmake_minimum_required(VERSION 3.10)

project(dots)

add_executable(${PROJECT_NAME}
  ${CMAKE_CURRENT_LIST_DIR}/src/main.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../../include
  ${CMAKE_CURRENT_LIST_DIR}/src
  /usr/include/SDL2
)

target_link_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../../lib
)

target_link_libraries(${PROJECT_NAME} jel SDL2)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# Windows compilation stuff
if (MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zc:preprocessor")
endif (MSVC)
