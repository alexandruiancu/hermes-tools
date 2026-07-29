#include "cuda_infer.hpp"
#include <cuda_runtime.h>
#include <iostream>
#include <fstream>

CudaInfer::CudaInfer(const std::string& model_path)
    : model_path_(model_path)
{
    // Load model file into memory
    std::ifstream fin(model_path_, std::ios::binary);
    if (!fin) {
        std::cerr << "Failed to open model file: " << model_path_ << "\n";
        return;
    }
    fin.seekg(0, std::ios::end);
    std::size_t sz = fin.tellg();
    fin.seekg(0, std::ios::beg);
    model_data_.resize(sz);
    fin.read(reinterpret_cast<char*>(model_data_.data()), sz);
    std::cout << "Loaded model of size " << sz << " bytes.\n";
}

CudaInfer::~CudaInfer() {}

std::vector<uint8_t> CudaInfer::run(const std::string& prompt, int width, int height)
{
    int total = width * height * 3;
    std::vector<uint8_t> host_buf(total);
    uint8_t* dev_buf;
    cudaMalloc(&dev_buf, total);
    // Copy model data into device buffer (truncated if larger)
    cudaMemcpy(dev_buf, model_data_.data(), std::min(total, (int)model_data_.size()), cudaMemcpyHostToDevice);
    // Copy back to host
    cudaMemcpy(host_buf.data(), dev_buf, total, cudaMemcpyDeviceToHost);
    cudaFree(dev_buf);
    return host_buf;
}
