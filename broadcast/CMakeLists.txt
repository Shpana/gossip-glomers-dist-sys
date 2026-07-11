add_executable(broadcast
    src/main.cpp
)

target_include_directories(broadcast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(broadcast PRIVATE core)