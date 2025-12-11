cmake_minimum_required(VERSION 3.16)

# using with Neovim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(ups LANGUAGES CXX)

# C++ version
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all .cpp files
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES})

# logger dependency
target_link_libraries(ups PRIVATE fmt)

# set binaries folder for output
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
)

# Symlink compile_commands.json from build to source dir (if it exists)
if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${PROJECT_SOURCE_DIR}/compile_commands.json"
    )
endif()
