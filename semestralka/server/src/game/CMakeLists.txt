file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(prsi_game_lib ${GAME_SOURCES})
target_include_directories(prsi_game_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
