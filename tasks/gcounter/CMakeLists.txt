add_executable(gcounter
  src/main.cpp
  src/operations.cpp
)

target_include_directories(gcounter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(gcounter PRIVATE maelstrom)