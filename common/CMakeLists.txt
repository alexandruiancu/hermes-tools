cmake_minimum_required(VERSION 3.20)

project(hermes_common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the library as a shared object
add_library(hermes_common SHARED
    src/server_utils.cpp
    src/config_loader.cpp
)

# Include directories
target_include_directories(hermes_common PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${COMMON_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link against Crow and nlohmann_json (both provided via CPM in the root CMakeLists)
target_link_libraries(hermes_common PUBLIC Crow::Crow nlohmann_json::nlohmann_json)
