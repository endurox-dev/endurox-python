cmake_minimum_required(VERSION 3.1)
project(endurox-python)
SET(CMAKE_BUILD_TYPE Debug)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/share/endurox/cmake")

include(ex_osver)

################################################################################
# Process any required includes
################################################################################
ex_osver_include()
################################################################################

# if using cmake directly...
if(NOT DEFINED NDRXPY_VERSION)
        set(NDRXPY_VERSION "0.0.0")
endif()

set(RELEASE "1")

if(DEFINED ENV{NDRX_BLD_RELEASE})
    set(RELEASE $ENV{NDRX_BLD_RELEASE})
endif()

message("CMake NDRXPY_VERSION  = ${NDRXPY_VERSION}")
message("CMake RELEASE  = ${RELEASE}")

# Configure versions
string(REPLACE "." ";" VERSION_LIST ${NDRXPY_VERSION})
list(GET VERSION_LIST 0 NDRXPY_VERSION_MAJOR)
list(GET VERSION_LIST 1 NDRXPY_VERSION_MINOR)
list(GET VERSION_LIST 2 NDRXPY_VERSION_PATCH)

MATH (EXPR NDRXPY_VERSION_NUM "${NDRXPY_VERSION_MAJOR} * 10000 + ${NDRXPY_VERSION_MINOR} * 100 + ${NDRXPY_VERSION_PATCH}")

set(NDRXPY_VERSION_MAJOR "\"${NDRXPY_VERSION_MAJOR}\"")
set(NDRXPY_VERSION_MINOR "\"${NDRXPY_VERSION_MINOR}\"")
set(NDRXPY_VERSION_PATCH "\"${NDRXPY_VERSION_PATCH}\"")


if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    # Find the python...
    find_package (Python3 COMPONENTS Development Interpreter)
else()
    set(PYTHON_EXECUTABLE, "python3")
endif()

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
                ${PYTHON_EXECUTABLE} --version
        RESULT_VARIABLE
                pyres
        OUTPUT_VARIABLE
                NDRXPY_PYVER
        )

if (NOT pyres EQUAL 0)
        message( FATAL_ERROR "Error getting python3 version" )
endif ()

if (NDRXPY_PYVER MATCHES "([0-9]+)\.([0-9]+)\.([0-9]+)")
    message(STATUS "Python Major: ${CMAKE_MATCH_1} Minor: ${CMAKE_MATCH_2}")
    set(PY_MAJOR ${CMAKE_MATCH_1})
    set(PY_MINOR ${CMAKE_MATCH_2})
else()
    message(FATAL_ERROR "Failed to extract python version")
endif()

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
        SET (NDRX_BLD_HASH      "unknown")
endif ()
################################################################################
ex_osver()
ex_cpuarch()

SET(CMAKE_CXX_FLAGS "-ggdb ${CMAKE_CXX_FLAGS}")
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED 99)

if (DEFINED ENV{NDRXPY_SANITIZE})
	SET(CMAKE_C_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O0 -ggdb ${CMAKE_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O0 -ggdb ${CMAKE_CXX_FLAGS}")
ELSE()
    # TODO: Solaris? AIX? Compilers?
    SET ( CMAKE_CXX_FLAGS "-O2 ${CMAKE_CXX_FLAGS}")
ENDIF()

if(CMAKE_OS_NAME STREQUAL "AIX")

    # Required as shared libs on AIX needs dependencies to be set (i.e. python shared lib)
    # and starting from 3.12 cmake, it has the Python package.
    if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
        message( FATAL_ERROR "CMake version 3.12 or greater is required, to find the Python3" )
    endif()

    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message("GNU compiler")
        set ( CMAKE_C_FLAGS "-D_SEM_SEMUN_UNDEFINED -D_MSGQSUPPORT -D_THREAD_SAFE -pthread -maix64 -Wl,-brtl,-bexpall,-berok ${CMAKE_C_FLAGS}")
        set ( CMAKE_CXX_FLAGS "-D_SEM_SEMUN_UNDEFINED -D_MSGQSUPPORT -D_THREAD_SAFE -pthread -maix64 -Wl,-brtl,-bexpall,-berok ${CMAKE_CXX_FLAGS}")
    elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        message("Clang Open XL compiler")
        set ( CMAKE_C_FLAGS "-D_SEM_SEMUN_UNDEFINED -D_MSGQSUPPORT -ftls-model=global-dynamic -D_THREAD_SAFE -pthread -brtl -bexpall -berok -Wno-unused-command-line-argument ${CMAKE_C_FLAGS}")
        set ( CMAKE_CXX_FLAGS "-D_SEM_SEMUN_UNDEFINED -D_MSGQSUPPORT -ftls-model=global-dynamic -D_THREAD_SAFE -pthread -brtl -bexpall -berok -Wno-unused-command-line-argument ${CMAKE_CXX_FLAGS}")
        set ( CMAKE_SHARED_LINKER_FLAGS "-shared -Wl,-G")
    elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "XL")
        message("XL compiler")
        SET ( CMAKE_CXX_FLAGS "-D_MSGQSUPPORT -O2 -qtls -q64 -b64 -qsuppress=1500-030" CACHE STRING "compile flags" FORCE)
        set ( CMAKE_C_FLAGS " -D_MSGQSUPPORT -D_SEM_SEMUN_UNDEFINED -D_THREAD_SAFE ")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qwarn64 -qpic -bexpfull -b64")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -brtl -qtls -q64")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qsuppress=1506-997 -qprocimported=setjmp")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qsuppress=1506-747")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qsuppress=1506-742")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qsuppress=1506-743")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qsuppress=1506-744")
        set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -qsuppress=1506-745")
        set ( CMAKE_SHARED_LINKER_FLAGS "-G")
        SET ( CMAKE_CXX_FLAGS "-D_MSGQSUPPORT -O2 -qtls -q64 -b64 -qsuppress=1500-030 -qnoinline" CACHE STRING "compile flags" FORCE)

        if (NOT DEFINE_RELEASEBUILD)
                message("XL compiler: with -g (debug symbols...)")
                set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
                set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
        endif()
    endif()
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(ATMI REQUIRED atmisrvinteg)

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

# Generate python module
add_subdirectory(pybind11)
add_subdirectory(tests/views)

# if this is AIX, then needs to add dependency libs
#pybind11_add_module(endurox SHARED ${SOURCES})

add_library(endurox MODULE ${SOURCES})
target_link_libraries(endurox PRIVATE pybind11::module pybind11::lto)

if(CMAKE_OS_NAME STREQUAL "AIX")
	if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
		# for GNU gcc built python versions, link to AIX std c++ lib will cause IOT/Abort trap (core dumped)
		target_link_libraries(endurox PRIVATE ${ATMI_LIBRARIES} ${Python3_LIBRARIES} "m")
	else()
		target_link_libraries(endurox PRIVATE ${ATMI_LIBRARIES} ${Python3_LIBRARIES} "c++" "m")
	endif()
else()
	target_link_libraries(endurox PRIVATE ${ATMI_LIBRARIES})
endif()

pybind11_extension(endurox)
set_target_properties(endurox PROPERTIES CXX_VISIBILITY_PRESET "hidden"
                                         CUDA_VISIBILITY_PRESET "hidden")


#
# Generate configuration 
#
configure_file ("${SOURCE_DIR}/ndrxpy_config.h.in"
                "${SOURCE_DIR}/ndrxpy_config.h" )


# Generate package file name 
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LO)

#
# Package release file
#
set(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME_LO}-${NDRXPY_VERSION}-${RELEASE}.${LSB_RELEASE_OUTPUT_OS}${LSB_RELEASE_OUTPUT_VER}_${CMAKE_CXX_COMPILER_ID}_cp${PY_MAJOR}${PY_MINOR}.${EX_CPU_ARCH}.whl.zip)

write_file("package_name.tmp" ${CPACK_PACKAGE_FILE_NAME})

message("Package name ${CPACK_PACKAGE_FILE_NAME} ${EX_CPU_ARCH}")

# vim: set ts=4 sw=4 et smartindent:
