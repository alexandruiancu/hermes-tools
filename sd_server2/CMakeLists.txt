cmake_minimum_required(VERSION 3.20)

project(sd_server2 LANGUAGES CXX)
set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler")

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
add_executable(sd2
    src/main.cpp
)

# Include directories
target_include_directories(sd2 PRIVATE include ${CMAKE_INCLUDE_PATH})

# Link libraries
target_link_libraries(sd2 PRIVATE CUDA::cudart Crow::Crow)
