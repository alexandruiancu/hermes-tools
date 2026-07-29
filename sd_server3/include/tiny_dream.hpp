#pragma once
#include <vector>
#include <string>

class TinyDream {
public:
    TinyDream();
    ~TinyDream();
    std::vector<uint8_t> run(const std::string& prompt, int width, int height);
};