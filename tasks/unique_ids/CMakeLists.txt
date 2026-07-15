add_executable(unique-ids
    src/main.cpp
)

target_include_directories(unique-ids PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unique-ids PRIVATE maelstrom)