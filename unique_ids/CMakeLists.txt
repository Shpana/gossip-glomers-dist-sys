add_executable(unique_ids
    src/main.cpp
)

target_include_directories(unique_ids PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(unique_ids PRIVATE core)