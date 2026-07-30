
CPMAddPackage(
    NAME llama.cpp
    GITHUB_REPOSITORY ggml-org/llama.cpp
    GIT_TAG b10107
    OPTIONS
    "LLAMA_CUDA=1"
)

# Build Crow server
add_executable(server src/main.cpp)

target_link_libraries(server PRIVATE Crow::Crow)
