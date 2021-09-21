cmake_minimum_required (VERSION 3.0)

file (GLOB_RECURSE test_srcs *.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../build
/usr/local/lib
)

#link_directories(/usr/local/lib)

set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -O0 -DDEBUG")

set(TEST_DEP_LIB mdm pthread gtest gmock gmock_main)

foreach(test_src IN LISTS test_srcs)
    string(REGEX REPLACE "(.*/)?(.*)\\.cpp$" "\\2" test_name ${test_src})
    string(FIND ${test_name} "test_" test_pos)
    if(${test_pos} EQUAL 0)
        list(APPEND tests ${test_name})
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} ${TEST_DEP_LIB})
        add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}> WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()
endforeach(test_src)
