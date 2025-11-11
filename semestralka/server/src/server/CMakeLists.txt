file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(prsi_server_lib ${SERVER_SOURCES})
target_include_directories(prsi_server_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(prsi_server_lib PUBLIC prsi_game_lib prsi_util_lib)
