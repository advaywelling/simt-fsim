cmake_minimum_required(VERSION 3.20)
project(simt_fsim CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sim src/main.cpp src/wave.cpp src/cfg.cpp)

target_include_directories(sim PRIVATE src)
target_compile_options(sim PRIVATE -Wall -Wextra -Wpedantic)

target_compile_options(sim PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined -g>)
target_link_options(sim PRIVATE $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
