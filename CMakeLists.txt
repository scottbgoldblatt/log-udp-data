cmake_minimum_required(VERSION 3.20)
project(hrg_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hrg_logger main.cpp)

# vcpkg installs component configs like boost_systemConfig.cmake
find_package(boost_system CONFIG REQUIRED)
target_link_libraries(hrg_logger PRIVATE Boost::system)

# (asio itself is header-only; if you want a find_package for consistency)
# find_package(boost_asio CONFIG REQUIRED)
# target_link_libraries(hrg_logger PRIVATE Boost::asio)