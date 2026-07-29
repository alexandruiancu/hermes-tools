#include "tiny_dream.hpp"
#include <vector>
#include <string>

TinyDream::TinyDream(){
    // In real code, load model files, allocate GPU memory, etc.
}

TinyDream::~TinyDream(){
    // Clean up resources
}

std::vector<uint8_t> TinyDream::run(const std::string& prompt, int width, int height){
    // Dummy: return a buffer of zeros
    int total = width * height * 3; // RGB
    return std::vector<uint8_t>(total, 0);
}
