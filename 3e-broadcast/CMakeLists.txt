add_executable(3e-broadcast
    src/main.cpp
    src/handlers/broadcast.cpp
    src/handlers/read.cpp
    src/handlers/topology.cpp
    src/workers/distributor.cpp
)

target_include_directories(3e-broadcast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(3e-broadcast PRIVATE core)