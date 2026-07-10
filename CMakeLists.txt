cmake_minimum_required(VERSION 3.22)
project(dist-sys CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG origin/main
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
)
FetchContent_MakeAvailable(json)

add_subdirectory(core)

add_subdirectory(echo)
