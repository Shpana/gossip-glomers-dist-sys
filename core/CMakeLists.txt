add_library(core STATIC 
    src/node.cpp
    src/messages.cpp
)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${json_SOURCE_DIR})
target_link_libraries(core PUBLIC nlohmann_json::nlohmann_json fmt::fmt)
