cmake_minimum_required(VERSION 3.10)
project(WebServer)

set(CMAKE_CXX_FLAGS "-O0 -Wno-deprecated -Wenum-compare -std=c++11")

#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O0")

add_subdirectory(WebServer)