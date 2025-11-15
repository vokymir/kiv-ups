cmake_minimum_required(VERSION 3.16)
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

# Include headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# Collect all .cpp files under src/
file(GLOB_RECURSE PRSI_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Main executable
add_executable(prsi_server ${PRSI_SOURCES})

# Optionally build tests
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
