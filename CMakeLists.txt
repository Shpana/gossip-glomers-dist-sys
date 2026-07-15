cmake_minimum_required(VERSION 3.22)
project(dist-sys CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG origin/main
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(yaclib
  GIT_REPOSITORY https://github.com/YACLib/YACLib.git
  GIT_TAG main
)
set(YACLIB_FLAGS "CORO" CACHE STRING "yaclib build flags")
FetchContent_MakeAvailable(yaclib)

# TODO(shpana): add configuration without tests
FetchContent_Declare(gtest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG main
)
FetchContent_MakeAvailable(gtest)

add_subdirectory(maelstrom)
add_subdirectory(tasks)