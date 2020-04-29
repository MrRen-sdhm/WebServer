cmake_minimum_required(VERSION 3.15)
project(WebServer)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(. SOURCE_FILES)

add_executable(WebServer ${SOURCE_FILES})

target_link_libraries(WebServer pthread)