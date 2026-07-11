add_library(core STATIC 
    src/network/messages.cpp
    src/network/network.cpp
    src/network/transport.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${json_SOURCE_DIR})
target_link_libraries(core PUBLIC yaclib nlohmann_json::nlohmann_json fmt::fmt)
