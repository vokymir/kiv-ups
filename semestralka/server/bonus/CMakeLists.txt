cmake_minimum_required(VERSION 4.1)
project("Bonus - starting point")

# C++ version
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# using with Neovim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(prsi.exe main.cpp server.cpp client_details.cpp)
