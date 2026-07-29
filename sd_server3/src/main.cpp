#include "cuda_infer.hpp"
#include <iostream>
#include <string>

int main(){
    CudaInfer infer("models/flux/flux1.safetensors");
    std::string prompt = "A cyberpunk city at night";
    auto img = infer.run(prompt, 512, 512);
    std::cout << "Generated image of size: " << img.size() << std::endl;
    return 0;
}
