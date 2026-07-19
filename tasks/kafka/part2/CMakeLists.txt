add_executable(kafka-part2
  src/main.cpp
  src/handlers.cpp
)

target_include_directories(kafka-part2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kafka-part2 PRIVATE maelstrom)