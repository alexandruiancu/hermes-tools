# Set default output directories for all targets in this project
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(COMMON_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/common/include")

cmake_minimum_required(VERSION 3.20)
project(HermesTools)

option(USE_CUDA "Enable CUDA support" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(USE_CUDA)
  message(STATUS "CUDA support enabled")
endif()

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG v3.12.0
)

CPMAddPackage("gh:chriskohlhoff/asio#asio-1-38-2@1-32-2")
if(asio_ADDED)
 add_library(asio::asio INTERFACE IMPORTED)
  target_include_directories(asio::asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
endif()

CPMAddPackage(
    NAME crow
    GITHUB_REPOSITORY crowcpp/crow
    GIT_TAG v1.3.3
    OPTIONS
        "CROW_BUILD_EXAMPLES=OFF"
        "CROW_BUILD_TESTS=OFF"
)

include_directories(${COMMON_INCLUDES})

# If you want to link against CUDA or other libraries, add them here
if(USE_CUDA)
  # Use modern FindCUDAToolkit
  find_package(CUDAToolkit REQUIRED)
  include_directories(${CUDAToolkit_INCLUDE_DIRS})
endif()

# ---------------------------------------------------------------------------
# Install rules
#
# `cmake --install build` (or `make install`) bundles everything the two
# servers need to run into a single directory:
#
#   <prefix>/bin/ai_router
#   <prefix>/bin/sd1
#   <prefix>/bin/config.json        (shared configuration for both)
#   <prefix>/bin/libhermes_common.so (runtime dep of both binaries)
#
# Override the destination with -DHERMES_INSTALL_DIR=<dir> if needed.
# ---------------------------------------------------------------------------
if(NOT DEFINED HERMES_INSTALL_DIR)
  set(HERMES_INSTALL_DIR "bin" CACHE STRING "Default installation root for Hermes")
endif()

add_subdirectory(common common/build)
add_subdirectory(router router/build)
add_subdirectory(sd_server1 sd_server1/build)

install(TARGETS ai_router sd1 RUNTIME DESTINATION ${HERMES_INSTALL_DIR})
install(TARGETS hermes_common LIBRARY DESTINATION ${HERMES_INSTALL_DIR})
install(FILES common/config.json DESTINATION ${HERMES_INSTALL_DIR})
install(DIRECTORY sd_server1/models DESTINATION ${HERMES_INSTALL_DIR})