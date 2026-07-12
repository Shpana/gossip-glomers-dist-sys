add_executable(3d-broadcast
    src/main.cpp
    src/handlers/broadcast.cpp
    src/handlers/read.cpp
    src/handlers/topology.cpp
    src/services/distributor.cpp
)

target_include_directories(3d-broadcast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(3d-broadcast PRIVATE core)