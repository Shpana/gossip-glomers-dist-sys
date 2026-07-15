add_executable(broadcast-part4
  src/main.cpp
  src/handlers/broadcast.cpp
  src/handlers/read.cpp
  src/handlers/topology.cpp
  src/workers/distributor.cpp
)

target_include_directories(broadcast-part4 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(broadcast-part4 PRIVATE maelstrom)