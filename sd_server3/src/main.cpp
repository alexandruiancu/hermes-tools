#include "cuda_infer.hpp"
#include <iostream>
#include <string>
#include <fstream>

int main(int argc, char** argv)
{
    // Load the model – path relative to the build dir
    CudaInfer infer("../models/flux/flux1.safetensors");

    // Prompt – from argv[1] or a default
    std::string prompt = (argc > 1) ? std::string(argv[1]) : "A cyberpunk city at night";

    // Run the dummy CUDA kernel – gets 512×512×3 bytes
    int width  = 512;
    int height = 512;
    auto img = infer.run(prompt, width, height);

    // Write the raw RGB buffer to a binary file
    std::ofstream out("output.rgb", std::ios::binary);
    out.write(reinterpret_cast<const char*>(img.data()), img.size());
    out.close();

    std::cout << "Wrote raw RGB: output.rgb (" << width << "x" << height << ")\n";
    return 0;
}
