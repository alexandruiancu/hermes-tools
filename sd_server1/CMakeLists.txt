cmake_minimum_required(VERSION 3.20)

project(sd_server1 LANGUAGES CXX)
set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler")

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CPM package manager (shared with the parent project)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CPM.cmake)

CPMAddPackage(
    NAME stable-diffusion
    GITHUB_REPOSITORY leejet/stable-diffusion.cpp
    GIT_TAG master
)

# Sources
add_executable(sd1
    src/main.cpp
)

# Include directories
target_include_directories(sd1 PRIVATE 
    include 
    ${CMAKE_INCLUDE_PATH}
    ${SD_INCLUDE_DIR}
)

# Link libraries
if(USE_CUDA)
  target_link_libraries(sd1 PRIVATE CUDA::cudart Crow::Crow hermes_common stable-diffusion)
  target_compile_definitions(sd1 PRIVATE USE_CUDA=1)
else()
  target_link_libraries(sd1 PRIVATE Crow::Crow hermes_common stable-diffusion)
endif()
