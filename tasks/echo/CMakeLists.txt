add_executable(echo 
    src/main.cpp
)

target_include_directories(echo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(echo PRIVATE maelstrom)