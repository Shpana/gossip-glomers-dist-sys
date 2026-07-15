add_library(maelstrom STATIC 
    src/network/messages.cpp
    src/network/network.cpp
    src/network/transport/console_transport.cpp
    src/network/transport/in_memory_transport.cpp

    src/detail/processors/network.cpp
    src/detail/sync/timer.cpp
)

target_include_directories(maelstrom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${json_SOURCE_DIR})
target_link_libraries(maelstrom PUBLIC yaclib nlohmann_json::nlohmann_json fmt::fmt)

# Testing
add_executable(echo-test ${CMAKE_CURRENT_SOURCE_DIR}/tests/echo_test.cpp)
add_executable(echo-deferred-test ${CMAKE_CURRENT_SOURCE_DIR}/tests/echo_deferred_test.cpp)
add_executable(init-test ${CMAKE_CURRENT_SOURCE_DIR}/tests/init_test.cpp)
add_executable(network-processor-test ${CMAKE_CURRENT_SOURCE_DIR}/tests/processors/network_test.cpp)

target_link_libraries(echo-test PRIVATE maelstrom GTest::gtest_main)
target_link_libraries(echo-deferred-test PRIVATE maelstrom GTest::gtest_main)
target_link_libraries(init-test PRIVATE maelstrom GTest::gtest_main)
target_link_libraries(network-processor-test PRIVATE maelstrom GTest::gtest_main)