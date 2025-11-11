file(GLOB_RECURSE UTIL_SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(prsi_util_lib ${UTIL_SOURCES})
target_include_directories(prsi_util_lib PUBLIC ${PROJECT_SOURCE_DIR}/include)
