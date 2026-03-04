cmake_minimum_required(VERSION 3.20)
project(hrg_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prefer Boost's CMake package config (works great with vcpkg)
find_package(Boost CONFIG REQUIRED COMPONENTS system)

add_executable(hrg_logger main.cpp)
target_link_libraries(hrg_logger PRIVATE Boost::system)