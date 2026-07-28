#include <fcgiapp.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

// Helper to run llama-diffusion-cli and capture its output
static std::string run_cli(const std::string &prompt, const std::string &extra = "") {
    std::string cmd = "/app/llama-diffusion-cli -m dream7b.gguf -p \"" + prompt + "\" -ub 512";
    if (!extra.empty()) cmd += " " + extra;
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("failed to run CLI");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
    // Open FastCGI connection
    if (FCGI_Accept() < 0) {
        std::cerr << "FastCGI accept failed" << std::endl;
        return 1;
    }

    // Read request body
    std::string body;
    char buf[4096];
    int n;
    while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
        body.append(buf, n);
    }

    // Very simple JSON parsing for "prompt"
    std::string prompt;
    auto ppos = body.find("\"prompt\":\"");
    if (ppos != std::string::npos) {
        ppos += 10;
        auto endp = body.find("\"", ppos);
        if (endp != std::string::npos) prompt = body.substr(ppos, endp - ppos);
    }

    // Optional params (steps, eps, algorithm) could be parsed similarly
    std::string extra;
    auto spos = body.find("\"steps\":");
    if (spos != std::string::npos) {
        spos += 8;
        auto endp = body.find(",", spos);
        if (endp == std::string::npos) endp = body.find("\"", spos);
        if (endp != std::string::npos) extra += " --diffusion-steps " + body.substr(spos, endp - spos);
    }
    auto epos = body.find("\"eps\":");
    if (epos != std::string::npos) {
        epos += 7;
        auto endp = body.find(",", epos);
        if (endp == std::string::npos) endp = body.find("\"", epos);
        if (endp != std::string::npos) extra += " --diffusion-eps " + body.substr(epos, endp - epos);
    }
    auto apos = body.find("\"algorithm\":");
    if (apos != std::string::npos) {
        apos += 13;
        auto endp = body.find(",", apos);
        if (endp == std::string::npos) endp = body.find("\"", apos);
        if (endp != std::string::npos) extra += " --diffusion-algorithm " + body.substr(apos, endp - apos);
    }

    std::string image_b64;
    try {
        image_b64 = run_cli(prompt, extra);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // Return HTTP response
    std::cout << "Content-Type: application/json\r\n\r\n";
    std::cout << "{\"image_base64\":\"" << image_b64 << "\"}";
    return 0;
}
