add_executable(3b-broadcast
    src/main.cpp
)

target_include_directories(3b-broadcast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(3b-broadcast PRIVATE maelstrom)