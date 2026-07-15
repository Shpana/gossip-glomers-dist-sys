add_executable(broadcast-part3
  src/main.cpp
)

target_include_directories(broadcast-part3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(broadcast-part3 PRIVATE maelstrom)