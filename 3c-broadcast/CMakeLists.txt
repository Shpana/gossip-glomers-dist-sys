add_executable(3c-broadcast
    src/main.cpp
)

target_include_directories(3c-broadcast PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(3c-broadcast PRIVATE core)