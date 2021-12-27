cmake_minimum_required (VERSION 3.0)

project (model)

# set default build type if not specified by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE debug)
endif()

set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-g -O0 -DDEBUG")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include 
/usr/local/include
)

link_directories(${mdm_BINARY_DIR} 
/usr/local/lib
)

set(mdm_headers ${CMAKE_CURRENT_SOURCE_DIR}/include/data_repo.h
${CMAKE_CURRENT_SOURCE_DIR}/include/data_parser.h
${CMAKE_CURRENT_SOURCE_DIR}/include/model_parser.h
${CMAKE_CURRENT_SOURCE_DIR}/include/common.h
${CMAKE_CURRENT_SOURCE_DIR}/include/macro.h
)

set(mdm_srcs ${CMAKE_CURRENT_SOURCE_DIR}/src/model_parser.c
${CMAKE_CURRENT_SOURCE_DIR}/src/data_parser.c
${CMAKE_CURRENT_SOURCE_DIR}/src/log.c
${CMAKE_CURRENT_SOURCE_DIR}/src/data_repo.c
${CMAKE_CURRENT_SOURCE_DIR}/src/common.c
) 

add_library(mdm SHARED ${mdm_srcs})
target_link_libraries(mdm cjson)
install(TARGETS mdm LIBRARY DESTINATION lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include/datapepo FILES_MATCHING PATTERN "*.h")

add_executable(model main.c)
target_link_libraries(model mdm cjson)

option(ENABLE_BUILD_TEST "Build tests" ON)

if(ENABLE_BUILD_TEST)
    enable_testing()
    add_subdirectory(./test tests)
endif()


