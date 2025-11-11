file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)
add_executable(prsi_tests ${TEST_SOURCES})
target_link_libraries(prsi_tests PRIVATE prsi_game_lib prsi_util_lib)

add_test(NAME prsi_tests COMMAND prsi_tests)
