cmake_minimum_required(VERSION 3.5)

project(shc)

option(CMAKE_BUILD_ASAN "Option for Address Sanitizer" OFF)

set(CMAKE_PROJECT_VERSION "SFC_V1.0R001")

set (CMAKE_CXX_FLAGS         "-std=c++11 -Wall -Werror -g")
set (CMAKE_CXX_FLAGS_RELEASE "-O2")
set (CMAKE_CXX_FLAGS_DEBUG   "-O0")

set (CMAKE_C_FLAGS         "-Wall -Werror -g")
# set (CMAKE_C_FLAGS         "-Wall -g")
set (CMAKE_C_FLAGS_RELEASE "-O2")
set (CMAKE_C_FLAGS_DEBUG   "-O0")

if (CMAKE_BUILD_ASAN)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

execute_process(
	COMMAND	git rev-parse --short HEAD
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
	OUTPUT_VARIABLE SVN_REVISION
)

execute_process(
	COMMAND date +%F-%Z/%T
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
	OUTPUT_VARIABLE BUILD_DATE
)

string (REGEX REPLACE "[\n\t\r]" "" SVN_REVISION ${SVN_REVISION})
string (REGEX REPLACE "[\n\t\r]" "" BUILD_DATE ${BUILD_DATE})

add_definitions( -DSVN_REVISION=\"${SVN_REVISION}\")
add_definitions( -DBUILD_DATE=\"${BUILD_DATE}\")
add_definitions( -DBUILD_VERSION=\"${CMAKE_PROJECT_VERSION}\")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

include(p4)
# include(spdlog)
# include(gflags)
# include(libfmt)
# add_subdirectory(common)
# add_subdirectory(agent)

add_subdirectory(platform)
