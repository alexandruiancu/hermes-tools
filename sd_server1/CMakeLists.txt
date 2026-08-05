
CPMAddPackage(
    NAME llama.cpp
    GITHUB_REPOSITORY ggml-org/llama.cpp
    GIT_TAG b10107
    OPTIONS
    "LLAMA_CUDA=1"
)

# Build Crow server
add_executable(sd1
    src/main.cpp)

target_link_libraries(sd1 PRIVATE Crow::Crow hermes_common)
