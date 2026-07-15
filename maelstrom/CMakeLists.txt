add_library(maelstrom STATIC 
    src/network/messages.cpp
    src/network/network.cpp
    src/network/transport.cpp

    src/detail/processors/network.cpp
    src/detail/sync/timer.cpp
)

target_include_directories(maelstrom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${json_SOURCE_DIR})
target_link_libraries(maelstrom PUBLIC yaclib nlohmann_json::nlohmann_json fmt::fmt)

add_executable(echo-tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/echo_tests.cpp)
target_link_libraries(echo-tests PRIVATE maelstrom GTest::gtest_main)