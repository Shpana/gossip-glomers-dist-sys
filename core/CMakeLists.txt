add_library(core STATIC src/maelstrom.cpp)

target_include_directories(core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(core PRIVATE folly Threads)
