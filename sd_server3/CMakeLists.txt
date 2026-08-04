cmake_minimum_required(VERSION 3.20)
project(sd_server3 LANGUAGES CXX)
set(CMAKE_CUDA_FLAGS "-allow-unsupported-compiler")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sd3 src/main.cpp src/cuda_infer.cpp)

# Download Flux model at build time
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/models/flux
        COMMAND curl -L -o ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors https://huggingface.co/anonymous/flux1.safetensors
    )
    add_custom_target(download_flux DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors)
    add_dependencies(sd3 download_flux)
endif()

# Include path for tiny_dream.hpp
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(sd3 PRIVATE CUDA::cudart)