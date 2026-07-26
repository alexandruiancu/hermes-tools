cmake_minimum_required(VERSION 3.22)
project(image_gen_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add package manager
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME llama.cpp
    GITHUB_REPOSITORY ggml-org/llama.cpp
    GIT_TAG b10107
    OPTIONS
    "LLAMA_CUDA=1"
)

CPMAddPackage("gh:chriskohlhoff/asio#asio-1-38-2@1-32-2")
if(asio_ADDED)
 add_library(asio::asio INTERFACE IMPORTED)
  target_include_directories(asio::asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
endif()
#target_link_libraries(asio::asio INTERFACE Threads::Threads)

CPMAddPackage(
    NAME crow
    GITHUB_REPOSITORY crowcpp/crow
    GIT_TAG v1.3.3
    OPTIONS
        "CROW_BUILD_EXAMPLES=OFF"
        "CROW_BUILD_TESTS=OFF"
)

## Find Crow (expects it to be installed system‑wide or vendored)
#find_package(Crow REQUIRED)

# Build Crow server
add_executable(server src/main.cpp)

target_link_libraries(server PRIVATE Crow::Crow)
