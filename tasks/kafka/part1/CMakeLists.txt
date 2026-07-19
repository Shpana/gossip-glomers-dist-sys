add_executable(kafka-part1
  src/main.cpp
  src/handlers.cpp
)

target_include_directories(kafka-part1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(kafka-part1 PRIVATE maelstrom)