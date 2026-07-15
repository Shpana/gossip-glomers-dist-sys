add_executable(broadcast-part1
  src/main.cpp
)

target_include_directories(broadcast-part1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(broadcast-part1 PRIVATE maelstrom)