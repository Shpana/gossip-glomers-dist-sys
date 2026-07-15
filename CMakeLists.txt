cmake_minimum_required(VERSION 3.22)
project(dist-sys CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-g")

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
FetchContent_MakeAvailable(yaclib)

set(YACLIB_FLAGS CORO)

add_subdirectory(core)

add_subdirectory(1-echo)
add_subdirectory(2-unique_ids)
add_subdirectory(3a-broadcast)
add_subdirectory(3b-broadcast)
add_subdirectory(3c-broadcast)
add_subdirectory(3d-broadcast)
add_subdirectory(3e-broadcast)
add_subdirectory(4-gcounter)