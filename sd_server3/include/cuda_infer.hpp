#pragma once
#include <vector>
#include <string>
#include <cstdint>

class CudaInfer {
public:
    CudaInfer(const std::string& model_path);
    ~CudaInfer();
    std::vector<uint8_t> run(const std::string& prompt, int width, int height);
private:
    std::string model_path_;
    std::vector<uint8_t> model_data_;
};
