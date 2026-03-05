# File: CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(imu_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(boost_system CONFIG REQUIRED)

add_executable(imu_logger
  src/main.cpp
  src/text_writer.cpp
  src/bin_writer.cpp
)

target_include_directories(imu_logger PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(imu_logger PRIVATE Boost::system)


add_executable(udp_test
  udp_test_sender_boost.cpp)

target_link_libraries(udp_test PRIVATE Boost::system)


# Put the binary in build/bin for convenience
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
endforeach()