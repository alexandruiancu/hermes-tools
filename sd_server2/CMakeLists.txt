cmake_minimum_required(VERSION 3.20)
project(sd_server LANGUAGES CXX CUDA)
set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler")

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# TODO
# Find CUDA
# Add Crow 

# Sources
add_executable(sd_server
    src/main.cpp
    src/cuda_infer.cpp
)

# Include directories
target_include_directories(sd_server PRIVATE include ${CMAKE_INCLUDE_PATH})

# Link libraries
target_link_libraries(sd_server PRIVATE CUDA::cudart Crow::crow)
