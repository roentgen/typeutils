cmake_minimum_required(VERSION 2.8)
include(CheckIncludeFileCXX)
# set(CMAKE_BUILD_TYPE Release)

project(typeutils)

set (MODULE typeutils)
set (EXECUTABLE_OUTPUT_PATH ./)

set(CMAKE_BUILD_TYPE_INIT Debug)
get_filename_component(THIRDPARTY ../ ABSOLUTE)

message(STATUS "Thirdparty:" ${THIRDPARTY})

check_include_file_cxx(cxxabi.h HAS_CXXABI_H)
if (HAS_CXXABI_H)
    add_definitions(-DHAS_CXXABI_H)	
endif()

if(WIN32)
    # C4127: use if constexpr
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /wd4819 /wd4458 /wd4127")
elseif(WIN32 OR UNIX)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14")
endif()



if(CMAKE_SIZEOF_VOIDP EQUAL 8)
    set(PLATFORM 64)
endif()

include_directories("include")

add_executable(recurse test/recurse.cpp include/aggregate.hpp)
add_executable(at test/at.cpp include/at.hpp)
add_executable(lu test/lu.cpp include/lookup.hpp include/aggregate.hpp)
add_executable(strings test/strings.cpp include/str.hpp)
add_executable(map test/map.cpp include/lookup.hpp include/aggregate.hpp include/str.hpp)
