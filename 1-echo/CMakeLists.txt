add_executable(1-echo 
    src/main.cpp
)

target_include_directories(1-echo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(1-echo PRIVATE core)