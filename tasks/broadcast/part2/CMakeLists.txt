add_executable(broadcast-part2
  src/main.cpp
)

target_include_directories(broadcast-part2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(broadcast-part2 PRIVATE maelstrom)