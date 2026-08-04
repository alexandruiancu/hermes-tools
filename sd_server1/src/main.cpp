#include <crow.h>
// Standard library headers needed for the CLI wrapper
#include <cstdio>      // popen, pclose, fgets
#include <cstdlib>     // std::runtime_error
#include <memory>      // std::unique_ptr
#include <string>      // std::string
#include <array>       // std::array

// Common helpers shared with other sd_server* projects
#include "server_utils.hpp"

// Run the llama-diffusion-cli binary and capture its stdout (base64 PNG)
std::string run_cli(const std::string &prompt, const std::string &extra = "") {
    std::string cmd = "/app/llama-diffusion-cli -m dream7b.gguf -p \"" + prompt + "\" -ub 512";
    if (!extra.empty()) cmd += " " + extra;
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("failed to run CLI");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result; // expected to be a base64 PNG string
}

int main() {
    crow::SimpleApp app;
    using namespace std::string_literals;

    CROW_ROUTE(app, "/generate").methods("POST"_method)([](const crow::request &req) {
        try {
            auto body = crow::json::load(req.body);
            ImageRequest reqs = ImageRequest::from_json(body);

            std::string image_b64 = run_cli(reqs.prompt, " --diffusion-steps " + std::to_string(reqs.steps));
            crow::json::wvalue resp;
            resp["image_base64"] = image_b64;
            return crow::response(resp);
        } catch (const std::exception &e) {
            return crow::response(500, e.what());
        }
    });

    app.port(8000).multithreaded().run();
    return 0;
}
