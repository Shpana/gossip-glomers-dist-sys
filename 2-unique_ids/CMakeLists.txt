add_executable(2-unique_ids
    src/main.cpp
)

target_include_directories(2-unique_ids PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(2-unique_ids PRIVATE core)