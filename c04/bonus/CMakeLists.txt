cmake_minimum_required(VERSION 4.1)
project("UPS Bonus 2")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bonus main.cpp server.cpp client_details.cpp)
