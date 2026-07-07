cmake_minimum_required(VERSION 3.22)
project(dist-sys CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(folly REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(core)