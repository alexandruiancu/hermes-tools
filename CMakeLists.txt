# Set default output directories for all targets in this project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(COMMON_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/common/include")

cmake_minimum_required(VERSION 3.20)
project(HermesTools)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(cmake/CPM.cmake)

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

# If you want to link against CUDA or other libraries, add them here
# Use modern FindCUDAToolkit
find_package(CUDAToolkit REQUIRED)
include_directories(${CUDAToolkit_INCLUDE_DIRS})
include_directories(${COMMON_INCLUDES})

add_subdirectory(common common/build)
add_subdirectory(sd_server1 sd_server1/build)
add_subdirectory(sd_server2 sd_server2/build)
add_subdirectory(sd_server3 sd_server3/build)