cmake_minimum_required(VERSION 3.1)
project(endurox-python)
SET(CMAKE_BUILD_TYPE Debug)


# if using cmake directly...
if(NOT DEFINED NDRXPY_VERSION)
	set(NDRXPY_VERSION "0.0.0")
endif()

message("CMake NDRXPY_VERSION  = ${NDRXPY_VERSION}")

# Configure versions
string(REPLACE "." ";" VERSION_LIST ${NDRXPY_VERSION})
list(GET VERSION_LIST 0 NDRXPY_VERSION_MAJOR)
list(GET VERSION_LIST 1 NDRXPY_VERSION_MINOR)
list(GET VERSION_LIST 2 NDRXPY_VERSION_PATCH)
MATH (EXPR NDRX_VERSION_NUM "${NDRXPY_VERSION_MAJOR} * 10000 + ${NDRXPY_VERSION_MINOR} * 100 + ${NDRXPY_VERSION_PATCH}")

message("CMake NDRXPY_VERSION_MINOR  = ${NDRXPY_VERSION_MINOR}")

################################################################################
# Check versions if defined, export build env
################################################################################

if(DEFINED ENV{NDRX_BLD_VERSION})
        IF(NOT ENV{NDRX_BLD_VERSION} MATCHES VERSION)
                message( FATAL_ERROR "Invalid versions: ENV: [$ENV{NDRX_BLD_VERSION}] Code: [${VERSION}]" )
        endif()
endif()

if(DEFINED ENV{NDRX_BLD_RELEASE})
    set(RELEASE $ENV{NDRX_BLD_RELEASE})
endif()

message("CMake RELEASE  = ${RELEASE}")

SET (NDRXPY_BLD_PRODUCT   $ENV{NDRX_BLD_PRODUCT})
SET (NDRXPY_BLD_SYSTEM    $ENV{NDRX_BLD_SYSTEM})
SET (NDRXPY_BLD_CONFIG    $ENV{NDRX_BLD_CONFIG})
SET (NDRXPY_BLD_VERSION   $ENV{NDRX_BLD_VERSION})
SET (NDRXPY_BLD_RELEASE   $ENV{NDRX_BLD_RELEASE})
SET (NDRXPY_BLD_TAG       $ENV{NDRX_BLD_TAG})
SET (NDRXPY_BLD_BRANCH    $ENV{NDRX_BLD_BRANCH})
SET (NDRXPY_BLD_COMMIT    $ENV{NDRX_BLD_COMMIT})
SET (NDRXPY_BLD_FLAGS     $ENV{NDRX_BLD_FLAGS})

#
# Get the git version (the version which is
#
execute_process(
        COMMAND
                git rev-parse HEAD
        RESULT_VARIABLE
                gitres
        OUTPUT_VARIABLE
                NDRXPY_BLD_HASH
        )

string(REGEX REPLACE "\n$" "" NDRXPY_BLD_HASH "${NDRXPY_BLD_HASH}")

if (NOT gitres EQUAL 0)
    SET (NDRXPY_BLD_HASH      "unknown")
endif ()

################################################################################

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
include_directories(${SOURCE_DIR}
			"${CMAKE_CURRENT_BINARY_DIR}/src/endurox")
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

#
# Generate configuration 
#
configure_file ("${SOURCE_DIR}/ndrxpy_config.h.in"
                "${SOURCE_DIR}/ndrxpy_config.h" )
