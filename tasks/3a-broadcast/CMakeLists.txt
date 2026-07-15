add_executable(3a-broadcast
    src/main.cpp
)

target_include_directories(3a-broadcast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(3a-broadcast PRIVATE maelstrom)