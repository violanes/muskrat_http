cmake_minimum_required(VERSION 3.25)
project(http_lib)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)

add_library(http_lib STATIC http_lib.hpp)
set_target_properties(http_lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(http_lib PUBLIC ${CURL_INCLUDE_DIRS})

add_executable(simple_post examples/simple_post.cpp)
target_link_libraries(simple_post ${CURL_LIBRARIES})

add_executable(simple_get examples/simple_get.cpp)
target_link_libraries(simple_get ${CURL_LIBRARIES})