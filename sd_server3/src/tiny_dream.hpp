#pragma once

#include <vector>
#include <string>
#include <cstdint>

class TinyDream {
public:
    TinyDream();
    ~TinyDream();
    std::vector<std::uint8_t> run(const std::string& prompt, int width, int height);
};
