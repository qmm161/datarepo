cmake_minimum_required (VERSION 2.6)

project (mdm)

# set default build type if not specified by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-g -O0 -DDEBUG")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(mdm_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/model.c) 

add_library(libmdm SHARED ${mdm_srcs})

add_executable(mdm main.c)

target_link_libraries(mdm libmdm)
