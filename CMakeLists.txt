cmake_minimum_required(VERSION 3.16)
project(jarvis_son VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-Wall -Wextra)

add_library(jarvis_core src/config.cpp src/proceso.cpp src/parser.cpp)
target_include_directories(jarvis_core PUBLIC include)

add_executable(jarvis src/main.cpp)
target_link_libraries(jarvis PRIVATE jarvis_core)

enable_testing()
add_executable(test_parser tests/test_parser.cpp)
target_link_libraries(test_parser PRIVATE jarvis_core)
add_test(NAME parser COMMAND test_parser)
