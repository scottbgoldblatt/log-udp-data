cmake_minimum_required(VERSION 3.20)
project(text_logger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# vcpkg installs component configs like boost_systemConfig.cmake

# (asio itself is header-only; if you want a find_package for consistency)
# find_package(boost_asio CONFIG REQUIRED)
# target_link_libraries(text_logger PRIVATE Boost::asio)cmake_minimum_required(VERSION 3.20)
project(text_logger LANGUAGES CXX)

# -- C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -- Options: build either or both
option(BUILD_TEXT_LOGGER "Build text (ascii) logger executable" ON)
option(BUILD_BIN_LOGGER  "Build binary logger executable"      ON)


# -- Sources
set(TEXT_LOGGER_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/text_logger.cpp)    # your boost/asio text file version
set(BIN_LOGGER_SRC   ${CMAKE_CURRENT_SOURCE_DIR}/bin_logger.cpp)     # the binary logger you asked for

# -- Find Boost (we need boost_system at least)
# Prefer the config-mode find_package (works well with vcpkg + exported targets)
find_package(boost_system CONFIG REQUIRED)

# If you also use Boost.Asio as a target in your environment, you can find it similarly.
# find_package(boost_asio CONFIG REQUIRED)

# -- Create targets based on options
if(BUILD_TEXT_LOGGER)
  add_executable(text_logger ${TEXT_LOGGER_SRC})
  target_compile_features(text_logger PRIVATE cxx_std_20)
  target_link_libraries(text_logger PRIVATE Boost::system)
  target_include_directories(text_logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}) # if you keep headers locally
  # install
  install(TARGETS text_logger RUNTIME DESTINATION bin)
endif()

if(BUILD_BIN_LOGGER)
  add_executable(bin_logger ${BIN_LOGGER_SRC})
  target_compile_features(bin_logger PRIVATE cxx_std_20)
  target_link_libraries(bin_logger PRIVATE Boost::system)
  target_include_directories(bin_logger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  install(TARGETS bin_logger RUNTIME DESTINATION bin)
endif()

add_executable(udp_test udp_test_sender_boost.cpp)

# Find Boost System (config-mode, works well with vcpkg)
find_package(boost_system CONFIG REQUIRED)
target_link_libraries(udp_test PRIVATE Boost::system)

# -- Useful: put executables in a single output dir for easy testing
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# If you want separate cfg dirs:
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
endforeach()

# -- Packaging / helpers (optional)
# Make a small target to show which executables were built
add_custom_target(show-build-info
  COMMAND ${CMAKE_COMMAND} -E echo "BUILD_TEXT_LOGGER=${BUILD_TEXT_LOGGER} BUILD_BIN_LOGGER=${BUILD_BIN_LOGGER}"
)



# If you use classic Boost packaging instead:
# find_package(Boost REQUIRED COMPONENTS system)
# target_link_libraries(udp_test_sender_boost PRIVATE Boost::system)

# -- CPack or install rules can be added below if desired