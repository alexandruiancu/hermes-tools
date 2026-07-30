cmake_minimum_required(VERSION 3.20)

#set(CMAKE_CUDA_COMPILER "/usr/local/cuda-13.3/bin/nvcc")   # adjust the path
#project(sd_server LANGUAGES CXX CUDA)

project(sd_server LANGUAGES CXX)
set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler")

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
add_executable(sd_server
    src/main.cpp
)

# Include directories
target_include_directories(sd_server PRIVATE include ${CMAKE_INCLUDE_PATH})

# Link libraries
target_link_libraries(sd_server PRIVATE CUDA::cudart Crow::Crow)
