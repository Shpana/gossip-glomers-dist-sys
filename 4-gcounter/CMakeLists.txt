add_executable(4-gcounter
    src/main.cpp
    src/ops.cpp
)

target_include_directories(4-gcounter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(4-gcounter PRIVATE core)