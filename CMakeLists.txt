cmake_minimum_required(VERSION 3.22)

project(seashell)

include(FetchContent)
FetchContent_Declare(serial
  GIT_REPOSITORY https://github.com/wjwwood/serial.git
  GIT_TAG v1.0.1
)
FetchContent_MakeAvailable(serial)

add_library(${PROJECT_NAME} src/seashell.c src/ulog.c)
target_include_directories(${PROJECT_NAME} PUBLIC src)

add_executable(basic examples/basic.cpp)
target_link_libraries(basic PUBLIC ${PROJECT_NAME} serial)