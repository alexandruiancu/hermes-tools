cmake_minimum_required(VERSION 3.20)
project(tinyDreamExample LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tiny_dream_example src/main.cpp src/cuda_infer.cpp)

# Download Flux model at build time
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/models/flux
        COMMAND curl -L -o ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors https://huggingface.co/anonymous/flux1.safetensors
    )
    add_custom_target(download_flux DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/models/flux/flux1.safetensors)
    add_dependencies(tiny_dream_example download_flux)
endif()

# Include path for tiny_dream.hpp
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find CUDA and include its headers
find_package(CUDA REQUIRED)
include_directories(${CUDA_INCLUDE_DIRS})
# If you want to link against CUDA or other libraries, add them here
target_link_libraries(tiny_dream_example PRIVATE ${CUDA_LIBRARIES})
