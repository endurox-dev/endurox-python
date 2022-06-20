cmake_minimum_required(VERSION 3.1)
project(endurox-python)
SET(CMAKE_BUILD_TYPE Debug)

if (DEFINED ENV{NDRXPY_SANITIZE})
	SET(CMAKE_C_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O0 -ggdb ${CMAKE_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O0 -ggdb ${CMAKE_CXX_FLAGS}")
ENDIF()

find_package(PkgConfig REQUIRED)
pkg_check_modules(ATMI REQUIRED atmisrvinteg)

SET(CMAKE_CXX_FLAGS "-ggdb ${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_STANDARD 11)

SET(SOURCE_DIR "src/endurox")
# Tell cmake that headers are in alse in source_dir
include_directories(${SOURCE_DIR})
SET(SOURCES 
	"${SOURCE_DIR}/endurox.cpp"
	"${SOURCE_DIR}/endurox_srv.cpp"
	"${SOURCE_DIR}/endurox_atmi.cpp"
	"${SOURCE_DIR}/atmibuf.cpp"
	"${SOURCE_DIR}/bufconv.cpp"
	"${SOURCE_DIR}/bufconv_view.cpp"
	"${SOURCE_DIR}/bufconv_ubf.cpp"
	"${SOURCE_DIR}/tpext.cpp"
	"${SOURCE_DIR}/tplog.cpp"
   )

#SET(TEST_DIR "tests")
#SET(TESTS ${SOURCES}
#    "${TEST_DIR}/test_main.cpp"
#    "${TEST_DIR}/test_math.cpp")

# Generate a test executable
include_directories(${SOURCE_DIR})
#add_executable("${PROJECT_NAME}_test" ${TESTS})

# Generate python module
add_subdirectory(pybind11)
add_subdirectory(tests/views)

pybind11_add_module(endurox ${SOURCES})
target_link_libraries(endurox PRIVATE ${ATMI_LIBRARIES})

# needed for testing
add_dependencies (endurox ubftestvhdrs)
