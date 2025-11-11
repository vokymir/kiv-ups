cmake_minimum_required(VERSION 4.1)
project("UPS - prší - javok")

# using with Neovim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# C++ version
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Enable warnings
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Submodules
add_subdirectory(src/game)
add_subdirectory(src/server)
add_subdirectory(src/util)

# Main executable
add_executable(prsi_server src/main.cpp)
target_link_libraries(prsi_server PRIVATE prsi_server_lib prsi_game_lib prsi_util_lib)

# Optionally build tests
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
